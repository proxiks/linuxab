/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/drivers/video/fbcon.h
 * Framebuffer Console
 */

#ifndef _LINUXAB_FBCON_H
#define _LINUXAB_FBCON_H

#include <stdint.h>

struct fbcon {
    uint32_t *fb_base;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t fg_color;
    uint32_t bg_color;
    uint32_t cursor_x;
    uint32_t cursor_y;
    uint32_t char_w;
    uint32_t char_h;
    uint32_t cols;
    uint32_t rows;
};

void fbcon_init(struct fbcon *con, uint32_t *base, uint32_t width, uint32_t height,
                uint32_t pitch, uint32_t bpp);
void fbcon_clear(struct fbcon *con);
void fbcon_putc(struct fbcon *con, char c);
void fbcon_puts(struct fbcon *con, const char *s);
void fbcon_set_color(struct fbcon *con, uint32_t fg, uint32_t bg);
void fbcon_scroll(struct fbcon *con);
void fbcon_draw_cursor(struct fbcon *con, bool visible);

#endif /* _LINUXAB_FBCON_H */
