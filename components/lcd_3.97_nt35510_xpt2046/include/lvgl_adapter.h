#ifndef _LVGL_ADAPTER_H_
#define _LVGL_ADAPTER_H_

#include "lvgl.h"

#define TAG "LVGL_ADAPTER"

#define I2S_PORT_NUM      (CONFIG_I2S_PORT_NUM)

#define LCD_D0_PIN        (CONFIG_LCD_D0_PIN)
#define LCD_D1_PIN        (CONFIG_LCD_D1_PIN)
#define LCD_D2_PIN        (CONFIG_LCD_D2_PIN)
#define LCD_D3_PIN        (CONFIG_LCD_D3_PIN)
#define LCD_D4_PIN        (CONFIG_LCD_D4_PIN)
#define LCD_D5_PIN        (CONFIG_LCD_D5_PIN)
#define LCD_D6_PIN        (CONFIG_LCD_D6_PIN)
#define LCD_D7_PIN        (CONFIG_LCD_D7_PIN)

#ifdef BIT_MODE_16BIT
    #define LCD_D8_PIN        (CONFIG_LCD_D8_PIN)
    #define LCD_D9_PIN        (CONFIG_LCD_D9_PIN)
    #define LCD_D10_PIN        (CONFIG_LCD_D10_PIN)
    #define LCD_D11_PIN        (CONFIG_LCD_D11_PIN)
    #define LCD_D12_PIN        (CONFIG_LCD_D12_PIN)
    #define LCD_D13_PIN        (CONFIG_LCD_D13_PIN)
    #define LCD_D14_PIN        (CONFIG_LCD_D14_PIN)
    #define LCD_D15_PIN        (CONFIG_LCD_D15_PIN)
#endif

#define LCD_WR_PIN        (CONFIG_LCD_WR_PIN)
#define LCD_RS_PIN        (CONFIG_LCD_RS_PIN)

#define LCD_RST_PIN        (CONFIG_LCD_RST_PIN)


#define TP_SPI_MOSI CONFIG_LV_TOUCH_SPI_MOSI
#define TP_SPI_MISO CONFIG_LV_TOUCH_SPI_MISO
#define TP_SPI_CLK  CONFIG_LV_TOUCH_SPI_CLK
#define TP_SPI_CS   CONFIG_LV_TOUCH_SPI_CS


#if defined (CONFIG_LV_TOUCH_CONTROLLER_SPI_HSPI)
#define TOUCH_SPI_HOST HSPI_HOST
#elif defined (CONFIG_LV_TOUCH_CONTROLLER_SPI_VSPI)
#define TOUCH_SPI_HOST VSPI_HOST
#elif defined (CONFIG_LV_TOUCH_CONTROLLER_SPI_FSPI)
#define TOUCH_SPI_HOST FSPI_HOST
#endif



void disp_init(void);
void disp_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);
void touch_init();
bool touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
bool lvgl_spi_driver_init(int host,
    int miso_pin, int mosi_pin, int sclk_pin,
    int max_transfer_sz,
    int dma_channel,
    int quadwp_pin, int quadhd_pin);
#endif