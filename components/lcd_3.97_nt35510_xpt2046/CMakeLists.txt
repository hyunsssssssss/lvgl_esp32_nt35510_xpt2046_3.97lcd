idf_component_register(
    SRCS "tp_spi.c" "tp_spi.c" "xpt2046.c" "i2s_lcd.c" "i2s_lcd_com.c" "nt35510.c" "lvgl_adapter.c"
    INCLUDE_DIRS "include"
    REQUIRES lvgl
)
