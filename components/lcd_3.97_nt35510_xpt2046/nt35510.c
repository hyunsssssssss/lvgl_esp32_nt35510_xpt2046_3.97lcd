// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "stdio.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "i2s_lcd_com.h"
#include "iot_nt35510.h"
#include "asc8x16.h"
#include "sdkconfig.h"

void iot_nt35510_set_orientation(nt35510_handle_t nt35510_handle, lcd_orientation_t orientation)
{
    uint16_t swap = 0;
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;

    // device->xset_cmd = NT35510_CASET;
    // device->yset_cmd = NT35510_RASET;

    switch (orientation) {
    case LCD_DISP_ROTATE_0:
        iot_i2s_lcd_write_reg(i2s_lcd_handle, NT35510_MADCTL, 0x00 | 0x00);
        break;
    case LCD_DISP_ROTATE_90:
        iot_i2s_lcd_write_reg(i2s_lcd_handle, NT35510_MADCTL, 0xA0 | 0x00);
        swap = device->x_size;
        device->x_size = device->y_size;
        device->y_size = swap;
        free(device->lcd_buf);
        device->lcd_buf = malloc(sizeof(uint16_t) * device->x_size);
        break;
    case LCD_DISP_ROTATE_180:
        iot_i2s_lcd_write_reg(i2s_lcd_handle, NT35510_MADCTL, 0xC0 | 0x00);
        break;
    case LCD_DISP_ROTATE_270:
        iot_i2s_lcd_write_reg(i2s_lcd_handle, NT35510_MADCTL, 0x60 | 0x00);
        swap = device->x_size;
        device->x_size = device->y_size;
        device->y_size = swap;
        free(device->lcd_buf);
        device->lcd_buf = malloc(sizeof(uint16_t) * device->x_size);
        break;
    default:
        iot_i2s_lcd_write_reg(i2s_lcd_handle, NT35510_MADCTL, 0x00 | 0x00);
        break;
    }
}

void iot_nt35510_set_box(nt35510_handle_t nt35510_handle, uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size)
{
    uint16_t x_end = x + (x_size - 1);
    uint16_t y_end = y + (y_size - 1);
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;
    iot_i2s_lcd_write_reg(i2s_lcd_handle, device->xset_cmd, x >> 8);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, device->xset_cmd + 1, x & 0xff);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, device->xset_cmd + 2, x_end >> 8);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, device->xset_cmd + 3, x_end & 0xff);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, device->yset_cmd, y >> 8);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, device->yset_cmd + 1, y & 0xff);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, device->yset_cmd + 2, y_end >> 8);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, device->yset_cmd + 3, y_end & 0xff);
    iot_i2s_lcd_write_cmd(i2s_lcd_handle, NT35510_RAMWR);
}

void iot_nt35510_refresh(nt35510_handle_t nt35510_handle)
{
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;
    iot_i2s_lcd_write(i2s_lcd_handle, device->lcd_buf, device->x_size * device->y_size * 2);
}

void iot_nt35510_fill_screen(nt35510_handle_t nt35510_handle, uint16_t color)
{
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;
    uint16_t *p = device->lcd_buf;
    iot_nt35510_set_box(nt35510_handle, 0, 0, device->x_size, device->y_size);
    for (int i = 0; i < device->x_size; i++) {
        p[i] = color;
    }
    for (int i = 0; i < device->y_size; i++) {
        iot_i2s_lcd_write(i2s_lcd_handle, device->lcd_buf, device->x_size * device->pix);
    }
}

void iot_nt35510_fill_area(nt35510_handle_t nt35510_handle, uint16_t color, uint16_t x, uint16_t y)
{
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;
    uint16_t *p = device->lcd_buf;
    for (int i = 0; i < x; i++) {
        p[i] = color;
    }
    for (int i = 0; i < y; i++) {
        iot_i2s_lcd_write(i2s_lcd_handle, device->lcd_buf, x * device->pix);
    }
}

void iot_nt35510_fill_rect(nt35510_handle_t nt35510_handle, uint16_t color, uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size)
{
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;
    uint16_t *p = device->lcd_buf;
    iot_nt35510_set_box(nt35510_handle, x, y, x_size, y_size);
    for(int i = 0; i < x_size; i++) {
        p[i] = color;
    }
    for(int i = 0; i < y_size; i++) {
        iot_i2s_lcd_write(i2s_lcd_handle, device->lcd_buf, x_size * device->pix);
    }
}

void iot_nt35510_draw_bmp(nt35510_handle_t nt35510_handle, uint16_t *bmp, uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size)
{
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;
    iot_nt35510_set_box(nt35510_handle, x, y, x_size, y_size);
    iot_i2s_lcd_write(i2s_lcd_handle, bmp, x_size * y_size * device->pix);
}

void iot_nt35510_put_char(nt35510_handle_t nt35510_handle, uint8_t *str, uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size, uint16_t wcolor, uint16_t bcolor)
{
    uint16_t *pbuf;
    uint8_t *pdata = str;
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    for (int i = 0; i < y_size; i++) {
        pbuf = device->lcd_buf + (x + (i + y) * device->x_size);
        for (int j = 0; j < x_size / 8; j++) {
            for (int k = 0; k < 8; k++) {
                if (*pdata & (0x80 >> k)) {
                    *pbuf = wcolor;
                } else {
                    *pbuf = bcolor;
                }
                pbuf++;
            }
            pdata++;
        }
    }
    iot_nt35510_refresh(nt35510_handle);
}

void iot_nt35510_asc8x16_to_men(nt35510_handle_t nt35510_handle, char str, uint16_t x, uint16_t y, uint16_t wcolor, uint16_t bcolor)
{
    uint16_t *pbuf;
    uint8_t *pdata = (uint8_t *)(font_asc8x16 + (str - ' ') * 16);
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    for (int i = 0; i < 16; i++) {
        pbuf = device->lcd_buf + (x + (i + y) * device->x_size);
        for (int k = 0; k < 8; k++) {
            if (*pdata & (0x80 >> k)) {
                *pbuf = wcolor;
            } else {
                *pbuf = bcolor;
            }
            pbuf++;
        }
        pdata++;
    }
}

void iot_nt35510_put_asc8x16(nt35510_handle_t nt35510_handle, char str, uint16_t x, uint16_t y, uint16_t wcolor, uint16_t bcolor)
{
    iot_nt35510_asc8x16_to_men(nt35510_handle, str, x, y, wcolor, bcolor);
    iot_nt35510_refresh(nt35510_handle);
}

void iot_nt35510_put_string8x16(nt35510_handle_t nt35510_handle, char *str, uint16_t x, uint16_t y, uint16_t wcolor, uint16_t bcolor)
{
    uint32_t x_ofsset = 0;
    uint32_t y_offset = 0;
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    while (*str != '\0') {
        iot_nt35510_asc8x16_to_men(nt35510_handle, *str, x + x_ofsset, y + y_offset, wcolor, bcolor);
        x_ofsset = x_ofsset + 8;
        if (x_ofsset > device->x_size - 8) {
            y_offset += 16;
            x_ofsset = 0;
            if (y_offset > device->y_size - 16) {
                break;
            }
        }
        str++;
    }
    iot_nt35510_refresh(nt35510_handle);
}

void iot_nt35510_reset(nt35510_handle_t nt35510_handle)
{
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;
    i2s_lcd_t *i2s_lcd = (i2s_lcd_t *)i2s_lcd_handle;
    gpio_set_level(i2s_lcd->i2s_lcd_conf.rst_io_num, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(i2s_lcd->i2s_lcd_conf.rst_io_num, 1);
    vTaskDelay(50 / portTICK_RATE_MS);
}

void iot_nt35510_init(nt35510_handle_t nt35510_handle)
{
    nt35510_dev_t *device = (nt35510_dev_t *)nt35510_handle;
    i2s_lcd_handle_t i2s_lcd_handle = device->i2s_lcd_handle;
    iot_nt35510_reset(nt35510_handle); // Reset before init
    
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF000, 0x55);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF001, 0xAA);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF002, 0x52);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF003, 0x08);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF004, 0x01);
    //# AVDD: manual, 
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB600, 0x34);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB601, 0x34);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB602, 0x34);

    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB000, 0x0D);//09
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB001, 0x0D);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB002, 0x0D);
    //# AVEE: manual,  -6V
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB700, 0x24);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB701, 0x24);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB702, 0x24);

    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB100, 0x0D);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB101, 0x0D);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB102, 0x0D);
    //#Power Control for
    //VCL
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB800, 0x24);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB801, 0x24);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB802, 0x24);

    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB200, 0x00);

    //# VGH: Clamp Enable, 
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB900, 0x24);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB901, 0x24);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB902, 0x24);

    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB300, 0x05);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB301, 0x05);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB302, 0x05);

    ///iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBF00, 0x01);

    //# VGL(LVGL):
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBA00, 0x34);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBA01, 0x34);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBA02, 0x34);
    //# VGL_REG(VGLO)
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB500, 0x0B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB501, 0x0B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB502, 0x0B);
    //# VGMP/VGSP:
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBC00, 0X00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBC01, 0xA3);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBC02, 0X00);
    //# VGMN/VGSN
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBD00, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBD01, 0xA3);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBD02, 0x00);
    //# VCOM=-0.1
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBE00, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBE01, 0x63);//4f
    //  VCOMH+0x01;
  //#R+
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD100, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD101, 0x37);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD102, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD103, 0x52);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD104, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD105, 0x7B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD106, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD107, 0x99);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD108, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD109, 0xB1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD10A, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD10B, 0xD2);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD10C, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD10D, 0xF6);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD10E, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD10F, 0x27);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD110, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD111, 0x4E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD112, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD113, 0x8C);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD114, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD115, 0xBE);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD116, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD117, 0x0B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD118, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD119, 0x48);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD11A, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD11B, 0x4A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD11C, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD11D, 0x7E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD11E, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD11F, 0xBC);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD120, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD121, 0xE1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD122, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD123, 0x10);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD124, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD125, 0x31);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD126, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD127, 0x5A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD128, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD129, 0x73);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD12A, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD12B, 0x94);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD12C, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD12D, 0x9F);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD12E, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD12F, 0xB3);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD130, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD131, 0xB9);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD132, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD133, 0xC1);
    //#G+
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD200, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD201, 0x37);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD202, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD203, 0x52);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD204, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD205, 0x7B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD206, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD207, 0x99);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD208, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD209, 0xB1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD20A, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD20B, 0xD2);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD20C, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD20D, 0xF6);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD20E, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD20F, 0x27);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD210, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD211, 0x4E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD212, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD213, 0x8C);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD214, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD215, 0xBE);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD216, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD217, 0x0B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD218, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD219, 0x48);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD21A, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD21B, 0x4A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD21C, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD21D, 0x7E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD21E, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD21F, 0xBC);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD220, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD221, 0xE1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD222, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD223, 0x10);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD224, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD225, 0x31);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD226, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD227, 0x5A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD228, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD229, 0x73);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD22A, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD22B, 0x94);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD22C, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD22D, 0x9F);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD22E, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD22F, 0xB3);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD230, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD231, 0xB9);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD232, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD233, 0xC1);
    //#B+
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD300, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD301, 0x37);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD302, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD303, 0x52);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD304, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD305, 0x7B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD306, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD307, 0x99);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD308, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD309, 0xB1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD30A, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD30B, 0xD2);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD30C, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD30D, 0xF6);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD30E, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD30F, 0x27);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD310, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD311, 0x4E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD312, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD313, 0x8C);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD314, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD315, 0xBE);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD316, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD317, 0x0B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD318, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD319, 0x48);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD31A, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD31B, 0x4A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD31C, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD31D, 0x7E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD31E, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD31F, 0xBC);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD320, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD321, 0xE1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD322, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD323, 0x10);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD324, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD325, 0x31);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD326, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD327, 0x5A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD328, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD329, 0x73);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD32A, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD32B, 0x94);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD32C, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD32D, 0x9F);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD32E, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD32F, 0xB3);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD330, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD331, 0xB9);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD332, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD333, 0xC1);

    //#R-///////////////////////////////////////////
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD400, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD401, 0x37);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD402, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD403, 0x52);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD404, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD405, 0x7B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD406, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD407, 0x99);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD408, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD409, 0xB1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD40A, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD40B, 0xD2);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD40C, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD40D, 0xF6);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD40E, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD40F, 0x27);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD410, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD411, 0x4E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD412, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD413, 0x8C);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD414, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD415, 0xBE);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD416, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD417, 0x0B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD418, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD419, 0x48);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD41A, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD41B, 0x4A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD41C, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD41D, 0x7E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD41E, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD41F, 0xBC);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD420, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD421, 0xE1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD422, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD423, 0x10);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD424, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD425, 0x31);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD426, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD427, 0x5A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD428, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD429, 0x73);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD42A, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD42B, 0x94);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD42C, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD42D, 0x9F);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD42E, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD42F, 0xB3);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD430, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD431, 0xB9);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD432, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD433, 0xC1);

    //#G-//////////////////////////////////////////////
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD500, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD501, 0x37);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD502, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD503, 0x52);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD504, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD505, 0x7B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD506, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD507, 0x99);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD508, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD509, 0xB1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD50A, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD50B, 0xD2);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD50C, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD50D, 0xF6);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD50E, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD50F, 0x27);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD510, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD511, 0x4E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD512, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD513, 0x8C);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD514, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD515, 0xBE);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD516, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD517, 0x0B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD518, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD519, 0x48);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD51A, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD51B, 0x4A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD51C, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD51D, 0x7E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD51E, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD51F, 0xBC);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD520, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD521, 0xE1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD522, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD523, 0x10);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD524, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD525, 0x31);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD526, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD527, 0x5A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD528, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD529, 0x73);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD52A, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD52B, 0x94);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD52C, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD52D, 0x9F);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD52E, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD52F, 0xB3);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD530, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD531, 0xB9);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD532, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD533, 0xC1);
    //#B-///////////////////////////////
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD600, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD601, 0x37);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD602, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD603, 0x52);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD604, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD605, 0x7B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD606, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD607, 0x99);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD608, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD609, 0xB1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD60A, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD60B, 0xD2);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD60C, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD60D, 0xF6);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD60E, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD60F, 0x27);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD610, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD611, 0x4E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD612, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD613, 0x8C);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD614, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD615, 0xBE);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD616, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD617, 0x0B);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD618, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD619, 0x48);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD61A, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD61B, 0x4A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD61C, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD61D, 0x7E);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD61E, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD61F, 0xBC);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD620, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD621, 0xE1);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD622, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD623, 0x10);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD624, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD625, 0x31);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD626, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD627, 0x5A);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD628, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD629, 0x73);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD62A, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD62B, 0x94);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD62C, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD62D, 0x9F);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD62E, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD62F, 0xB3);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD630, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD631, 0xB9);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD632, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xD633, 0xC1);



    //#Enable Page0
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF000, 0x55);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF001, 0xAA);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF002, 0x52);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF003, 0x08);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xF004, 0x00);
    //# RGB I/F Setting
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB000, 0x08);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB001, 0x05);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB002, 0x02);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB003, 0x05);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB004, 0x02);
    //## SDT:
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB600, 0x08);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB500, 0x50);//0x6b ???? 480x854       0x50 ???? 480x800

    //## Gate EQ:
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB700, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB701, 0x00);

    //## Source EQ:
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB800, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB801, 0x05);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB802, 0x05);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xB803, 0x05);

    //# Inversion: Column inversion (NVT)
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBC00, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBC01, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBC02, 0x00);

    //# BOE's Setting(default)
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xCC00, 0x03);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xCC01, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xCC02, 0x00);

    //# Display Timing:
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBD00, 0x01);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBD01, 0x84);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBD02, 0x07);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBD03, 0x31);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBD04, 0x00);

    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xBA00, 0x01);

    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xFF00, 0xAA);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xFF01, 0x55);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xFF02, 0x25);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0xFF03, 0x01);

    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0x3500, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0x3600, 0x00);
    iot_i2s_lcd_write_reg(i2s_lcd_handle, 0x3a00, 0x55);  ////55=16?/////66=18?
    iot_i2s_lcd_write_cmd(i2s_lcd_handle, 0x1100); // Out sleep

    vTaskDelay(120 / portTICK_RATE_MS);
    //Display On
    iot_i2s_lcd_write_cmd(i2s_lcd_handle, 0x2900);
    // Write continue
    iot_i2s_lcd_write_cmd(i2s_lcd_handle, 0x2C00);

}

nt35510_handle_t iot_nt35510_create(uint16_t x_size, uint16_t y_size, i2s_port_t i2s_port, i2s_lcd_config_t *pin_conf)
{
    nt35510_dev_t *device = NULL;
    device = (nt35510_dev_t *)malloc(sizeof(nt35510_dev_t));
    if (device == NULL) {
        LCD_LOG("LCD dev malloc fail\n");
        goto error;
    }
    memset(device, 0, sizeof(nt35510_dev_t));
    device->x_size = x_size;
    device->y_size = y_size;
    device->xset_cmd = NT35510_CASET;
    device->yset_cmd = NT35510_RASET;
    device->pix = sizeof(uint16_t);
    uint16_t *p = malloc(sizeof(uint16_t) * x_size);
    if (p == NULL) {
        LCD_LOG("malloc fail\n");
        goto error;
    }
    device->lcd_buf = p;
    device->i2s_lcd_handle = iot_i2s_lcd_pin_cfg(i2s_port, pin_conf);
    iot_nt35510_init((nt35510_handle_t)device);
    return (nt35510_handle_t)device;

error:
    if (device) {
        free(device);
    }
    return NULL;
}