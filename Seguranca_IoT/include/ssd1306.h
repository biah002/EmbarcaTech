#ifndef SSD1306_H
#define SSD1306_H

#include "hardware/i2c.h"

typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
    uint8_t width;
    uint8_t height;
    uint8_t buffer[1024];
} ssd1306_t;

void ssd1306_init(ssd1306_t *disp, uint8_t width, uint8_t height, i2c_inst_t *i2c, uint8_t addr);
void ssd1306_clear(ssd1306_t *disp);
void ssd1306_show(ssd1306_t *disp);
void ssd1306_draw_pixel(ssd1306_t *disp, int x, int y);
void ssd1306_draw_rect(ssd1306_t *disp, int x, int y, int w, int h);
void ssd1306_fill_rect(ssd1306_t *disp, int x, int y, int w, int h);
void ssd1306_draw_string(ssd1306_t *disp, int x, int y, int scale, const char *str);

#endif