#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "esp_system.h"

#include "lvgl.h"
#include "lvgl_adapter.h"
#if defined CONFIG_LV_USE_DEMO_WIDGETS
    #include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"
#elif defined CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER
    #include "lv_examples/src/lv_demo_keypad_and_encoder/lv_demo_keypad_and_encoder.h"
#elif defined CONFIG_LV_USE_DEMO_BENCHMARK
    #include "lv_examples/src/lv_demo_benchmark/lv_demo_benchmark.h"
#elif defined CONFIG_LV_USE_DEMO_STRESS
    #include "lv_examples/src/lv_demo_stress/lv_demo_stress.h"
#else
    #error "No demo application selected."
#endif

#define LV_TICK_PERIOD_MS 1
#define DISP_BUF_SIZE 800*40

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

static void create_demo_application(void)
{
    #if defined CONFIG_LV_USE_DEMO_WIDGETS
        lv_demo_widgets();
    #elif defined CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER
        lv_demo_keypad_encoder();
    #elif defined CONFIG_LV_USE_DEMO_BENCHMARK
        lv_demo_benchmark();
    #elif defined CONFIG_LV_USE_DEMO_STRESS
        lv_demo_stress();
    #else
        #error "No demo application selected."
    #endif
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void guiTask(void *pvParameter) {
    
    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();
    
    disp_init();
    touch_init();
    
    static lv_color_t buf1[DISP_BUF_SIZE];
    static lv_color_t buf2[DISP_BUF_SIZE];

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    // #if defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_IL3820
    //     || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_JD79653A
    //     || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_UC8151D
        
    //     /* Actual size in pixels, not bytes. */
    //     size_in_px *= 8;
    // #endif

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */
    create_demo_application();
    
    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    vTaskDelete(NULL);
}

void app_main(void)
{   
    /* If you want to use a task to create the graphic, you NEED to create a Pinned task
     * Otherwise there can be problem such as memory corruption and so on.
     * NOTE: When not using Wi-Fi nor Bluetooth you can pin the guiTask to core 0 */
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
    
}
