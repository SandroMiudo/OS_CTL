#include "frame_buffer.h"

static void fb_clear(fb_info_ptr fb, uint32_t to) {
    for (int i = 0; i < fb->width * fb->height; i++)
        ((uint32_t *)fb->buffer)[i] = to;
}

static void fb_set_white(fb_info_ptr fb) {
    fb_clear(fb, 0x00FFFFFF);
}

static void fb_set_black(fb_info_ptr fb) {
    fb_clear(fb, 0x00000000);
}

static void (*f[2])(fb_info_ptr) = { [0] = fb_set_white, [1] = fb_set_black };

void fill_command(FrameBufferFillCommands comm, fb_info_ptr fb) {
    f[comm](fb);
}

void fb_set_pixel(fb_info_ptr fb, int x, int y, uint32_t color) {
    if (!fb || x < 0 || x >= fb->width || y < 0 || y >= fb->height) return;

    uint8_t *row_start = fb->buffer + y * fb->stride;
    uint32_t *pixel = (uint32_t *)(row_start + x * fb->bpp);

    *pixel = color;
}

void fb_draw_image(fb_info_ptr fb, int x0, int y0,
                   uint32_t img_width, uint32_t img_height,
                   const uint32_t *image) {
    if (!fb || !image) return;

    for (uint32_t y = 0; y < img_height; y++) {
        if ((y0 + y) >= fb->height) break; // clip bottom
        for (uint32_t x = 0; x < img_width; x++) {
            if ((x0 + x) >= fb->width) break; // clip right

            uint32_t *pixel_ptr = (uint32_t *)(fb->buffer + (y0 + y) * fb->stride + (x0 + x) * fb->bpp);
            *pixel_ptr = image[y * img_width + x];
        }
    }
}