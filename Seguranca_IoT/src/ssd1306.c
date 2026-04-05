#include "ssd1306.h"
#include <string.h>

// Tabela de caracteres 5x7 (Simplificada para teste rápido)
static const uint8_t font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // (espaço) - 0
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0 - 1
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1 - 2
    0x42, 0x61, 0x51, 0x49, 0x46, // 2 - 3
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3 - 4
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4 - 5
    0x27, 0x45, 0x45, 0x45, 0x39, // 5 - 6
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6 - 7
    0x01, 0x71, 0x09, 0x05, 0x03, // 7 - 8
    0x36, 0x49, 0x49, 0x49, 0x36, // 8 - 9
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9 - 10
    0x7C, 0x12, 0x11, 0x12, 0x7C, // A - 11
    0x7F, 0x49, 0x49, 0x49, 0x36, // B - 12
    0x3E, 0x41, 0x41, 0x41, 0x22, // C - 13
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D - 14
    0x7F, 0x49, 0x49, 0x49, 0x41, // E - 15
    0x7F, 0x09, 0x09, 0x09, 0x01, // F - 16
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G - 17
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H - 18
    0x00, 0x41, 0x7F, 0x41, 0x00, // I - 19
    0x20, 0x40, 0x41, 0x3F, 0x01, // J - 20
    0x7F, 0x08, 0x14, 0x22, 0x41, // K - 21
    0x7F, 0x40, 0x40, 0x40, 0x40, // L - 22
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M - 23
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N - 24
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O - 25
    0x7F, 0x09, 0x09, 0x09, 0x06, // P - 26
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q - 27
    0x7F, 0x09, 0x19, 0x29, 0x46, // R - 28
    0x46, 0x49, 0x49, 0x49, 0x31, // S - 29
    0x01, 0x01, 0x7F, 0x01, 0x01, // T - 30
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U - 31
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V - 32
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W - 33
    0x63, 0x14, 0x08, 0x14, 0x63, // X - 34
    0x07, 0x08, 0x70, 0x08, 0x07, // Y - 35
    0x61, 0x51, 0x49, 0x45, 0x43, // Z - 36
    0x00, 0x36, 0x36, 0x00, 0x00  // : - 37
};

static void cmd(ssd1306_t *d, uint8_t c) {
    uint8_t buf[2] = {0x00, c};
    i2c_write_blocking(d->i2c, d->addr, buf, 2, false);
}

void ssd1306_init(ssd1306_t *d, uint8_t w, uint8_t h, i2c_inst_t *i2c, uint8_t addr) {
    d->i2c = i2c; d->addr = addr; d->width = w; d->height = h;
    sleep_ms(100);
    cmd(d, 0xAE); cmd(d, 0x20); cmd(d, 0x00); cmd(d, 0xB0); cmd(d, 0xC8);
    cmd(d, 0x00); cmd(d, 0x10); cmd(d, 0x40); cmd(d, 0x81); cmd(d, 0xFF);
    cmd(d, 0xA1); cmd(d, 0xA6); cmd(d, 0xA8); cmd(d, 0x3F); cmd(d, 0xA4);
    cmd(d, 0xD3); cmd(d, 0x00); cmd(d, 0xD5); cmd(d, 0xF0); cmd(d, 0xD9);
    cmd(d, 0x22); cmd(d, 0xDA); cmd(d, 0x12); cmd(d, 0xDB); cmd(d, 0x20);
    cmd(d, 0x8D); cmd(d, 0x14); cmd(d, 0xAF);
    ssd1306_clear(d);
    ssd1306_show(d);
}

void ssd1306_clear(ssd1306_t *d) {
    memset(d->buffer, 0, 1024);
}

void ssd1306_show(ssd1306_t *d) {
    for (uint8_t i = 0; i < 8; i++) {
        cmd(d, 0xB0 + i);
        cmd(d, 0x00);
        cmd(d, 0x10);
        uint8_t temp[129];
        temp[0] = 0x40;
        memcpy(&temp[1], &d->buffer[i * 128], 128);
        i2c_write_blocking(d->i2c, d->addr, temp, 129, false);
    }
}

void ssd1306_draw_pixel(ssd1306_t *d, int x, int y) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    d->buffer[x + (y / 8) * 128] |= (1 << (y % 8));
}

void ssd1306_fill_rect(ssd1306_t *d, int x, int y, int w, int h) {
    for (int i = x; i < x + w; i++)
        for (int j = y; j < y + h; j++)
            ssd1306_draw_pixel(d, i, j);
}

void ssd1306_draw_rect(ssd1306_t *d, int x, int y, int w, int h) {
    for (int i = x; i < x + w; i++) { ssd1306_draw_pixel(d, i, y); ssd1306_draw_pixel(d, i, y + h - 1); }
    for (int j = y; j < y + h; j++) { ssd1306_draw_pixel(d, x, j); ssd1306_draw_pixel(d, x + w - 1, j); }
}

void ssd1306_draw_char(ssd1306_t *d, int x, int y, int scale, char c) {
    uint8_t idx = 0;
    if (c >= '0' && c <= '9') idx = c - '0' + 1;
    else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
    else if (c == ':') idx = 37;
    else if (c == ' ') idx = 0;

    for (int i = 0; i < 5; i++) {
        uint8_t line = font[idx * 5 + i];
        for (int j = 0; j < 8; j++) {
            if (line & (1 << j)) {
                if (scale == 1) ssd1306_draw_pixel(d, x + i, y + j);
                else ssd1306_fill_rect(d, x + i * scale, y + j * scale, scale, scale);
            }
        }
    }
}

void ssd1306_draw_string(ssd1306_t *d, int x, int y, int scale, const char *str) {
    while (*str) {
        ssd1306_draw_char(d, x, y, scale, *str);
        x += 8 * scale;
        str++;
    }
}