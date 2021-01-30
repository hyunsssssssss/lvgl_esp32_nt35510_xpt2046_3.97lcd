/* RTOS Includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "tp_spi.h"
#include "xpt2046.h"
#include "iot_nt35510.h"
#include "lvgl_adapter.h"


nt35510_handle_t nt35510_handle;

SemaphoreHandle_t flush_sem = NULL;
static lv_disp_drv_t* flush_drv;
static uint16_t flush_width;
static uint16_t flush_height;
static uint16_t flush_offset_x;
static uint16_t flush_offset_y;
static uint16_t *pFrameBuffer = NULL;

void board_lcd_flush_task(void *arg)
{
    portBASE_TYPE res;
    while (1) {
        res = xSemaphoreTake(flush_sem, portMAX_DELAY);
        if (res == pdTRUE) {
            iot_nt35510_draw_bmp(nt35510_handle, (uint16_t *)pFrameBuffer, flush_offset_x, flush_offset_y, flush_width, flush_height);
            lv_disp_flush_ready(flush_drv);
            // vTaskDelay(100 / portTICK_RATE_MS);
        }
    }
}

void disp_init(void) {
    i2s_lcd_config_t i2s_lcd_pin_conf = {
        #ifdef CONFIG_BIT_MODE_8BIT
        .data_width = 8,
        .data_io_num = {
            LCD_D0_PIN,  LCD_D1_PIN,  LCD_D2_PIN,  LCD_D3_PIN,
            LCD_D4_PIN,  LCD_D5_PIN,  LCD_D6_PIN,  LCD_D7_PIN,
        },
        #else // CONFIG_BIT_MODE_16BIT
        .data_width = 16,
        .data_io_num = {
            LCD_D0_PIN,  LCD_D1_PIN,  LCD_D2_PIN,  LCD_D3_PIN,
            LCD_D4_PIN,  LCD_D5_PIN,  LCD_D6_PIN,  LCD_D7_PIN,
            LCD_D8_PIN,  LCD_D9_PIN,  LCD_D10_PIN, LCD_D11_PIN,
            LCD_D12_PIN, LCD_D13_PIN, LCD_D14_PIN, LCD_D15_PIN,
        },
        #endif
        .ws_io_num = LCD_WR_PIN,
        .rs_io_num = LCD_RS_PIN,
        .rst_io_num = LCD_RST_PIN,
    };

    nt35510_handle = iot_nt35510_create(480, 800, I2S_PORT_NUM, &i2s_lcd_pin_conf);

    if (nt35510_handle == NULL) {
        LCD_LOG("nt35510 create fail!\n");
        return;
    }

    iot_nt35510_set_orientation(nt35510_handle, LCD_DISP_ROTATE_90);
    iot_nt35510_fill_screen(nt35510_handle, 0x0000);

    // For framebuffer mode and flush
    if (flush_sem == NULL) {
        flush_sem = xSemaphoreCreateBinary();
    }
    xTaskCreate(board_lcd_flush_task, "flush_task", 1500, NULL, 5, NULL);

    LCD_LOG("nt35510 init ok!\n");
}

void disp_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map) {
    flush_drv = drv;
    flush_offset_x = area->x1;
    flush_offset_y = area->y1;
    flush_width = area->x2 - flush_offset_x + 1;
    flush_height = area->y2 - flush_offset_y + 1;
    pFrameBuffer = (uint16_t*)color_map;
    xSemaphoreGive(flush_sem);
}

void touch_init() {
    ESP_LOGI(TAG, "Initializing SPI master for touch");

    lvgl_spi_driver_init(TOUCH_SPI_HOST,
        TP_SPI_MISO, TP_SPI_MOSI, TP_SPI_CLK,
        0 /* Defaults to 4094 */, 2,
        -1, -1);

    tp_spi_add_device(TOUCH_SPI_HOST);

    xpt2046_init();
}

bool touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    bool res = false;
    res = xpt2046_read(drv, data);
    return res;
}

/* Initialize spi bus master */
bool lvgl_spi_driver_init(int host,
    int miso_pin, int mosi_pin, int sclk_pin,
    int max_transfer_sz,
    int dma_channel,
    int quadwp_pin, int quadhd_pin)
{
#if defined (CONFIG_IDF_TARGET_ESP32)
    assert((SPI_HOST <= host) && (VSPI_HOST >= host));
    const char *spi_names[] = {
        "SPI_HOST", "HSPI_HOST", "VSPI_HOST"
    };
#elif defined (CONFIG_IDF_TARGET_ESP32S2)
    assert((SPI_HOST <= host) && (HSPI_HOST >= host));
    const char *spi_names[] = {
        "SPI_HOST", "", ""
    };
#endif

    ESP_LOGI(TAG, "Configuring SPI host %s (%d)", spi_names[host], host);
    ESP_LOGI(TAG, "MISO pin: %d, MOSI pin: %d, SCLK pin: %d, IO2/WP pin: %d, IO3/HD pin: %d",
        miso_pin, mosi_pin, sclk_pin, quadwp_pin, quadhd_pin);

    ESP_LOGI(TAG, "Max transfer size: %d (bytes)", max_transfer_sz);

    spi_bus_config_t buscfg = {
        .miso_io_num = miso_pin,
	.mosi_io_num = mosi_pin,
	.sclk_io_num = sclk_pin,
	.quadwp_io_num = quadwp_pin,
	.quadhd_io_num = quadhd_pin,
        .max_transfer_sz = max_transfer_sz
    };

    ESP_LOGI(TAG, "Initializing SPI bus...");
    esp_err_t ret = spi_bus_initialize(host, &buscfg, dma_channel);
    assert(ret == ESP_OK);

    return ESP_OK != ret;
}
