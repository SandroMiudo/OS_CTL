/*
Draw simple face, only using set pixel command
*/

#include <stdint.h>
#include "display_api.h"
#include <stddef.h>

// API WRAPPER
static void set_pixel(unsigned int x, unsigned int y, uint32_t color) {
    cmd_set_pixel_t cmd = { x, y, color };
    display_set_pixel(&cmd, 0, NULL, NULL);
}

static void fill_screen(uint32_t color) {
    cmd_fill_screen_t cmd = { color };
    display_fill_screen(&cmd, 0, NULL, NULL);
}

static void clear_screen(uint8_t on) {
    cmd_clear_screen_t cmd = { on };
    display_set_pixel(&cmd, 0, NULL, NULL);
}

// --------------------------------------------------
// Draw a filled circle
// --------------------------------------------------
static void draw_circle(unsigned int cx, unsigned int cy, unsigned int r, uint32_t color)
{
    for (int y = -(int)r; y <= (int)r; y++) {
        for (int x = -(int)r; x <= (int)r; x++) {
            if (x*x + y*y <= (int)r * (int)r) {
                set_pixel(cx + x, cy + y, color);
            }
        }
    }
}

int main(void) {
    const unsigned int W = 520;
    const unsigned int H = 520;

    // Colors (ARGB format)
    const uint32_t _WHITE  = 0xFFFFFFFF;
    const uint32_t _BLACK  = 0xFF000000;
    const uint32_t SKIN   = 0xFFFFCBA0;
    const uint32_t PINK   = 0xFFFF8FA0;
    const uint32_t BLUE   = 0xFF3A7BD5;

    const uint32_t BG = 0xFFE6F0FF;

    // Background (white), uncomment for manual drawing each pixel (takes a bit)
    /* for (unsigned int y = 0; y < H; y++) {
        for (unsigned int x = 0; x < W; x++) {
            set_pixel(x, y, _WHITE);
        }
    }*/

    clear_screen(WHITE);

    fill_screen(BG);

    // Face (large circle)
    draw_circle(260, 260, 200, SKIN);

    // Eyes
    draw_circle(190, 220, 35, _WHITE);
    draw_circle(330, 220, 35, _WHITE);

    // Pupils
    draw_circle(190, 220, 12, _BLACK);
    draw_circle(330, 220, 12, _BLACK);

    // Eye shine
    draw_circle(185, 215, 5, _WHITE);
    draw_circle(325, 215, 5, _WHITE);

    // Blush
    draw_circle(140, 300, 25, PINK);
    draw_circle(380, 300, 25, PINK);

    // Mouth (simple line)
    for (int x = 200; x <= 320; x++) {
        set_pixel(x, 330, _BLACK);
    }

    // Add small smile curve
    for (int i = 0; i < 30; i++) {
        set_pixel(200 + i, 335 + i / 10, _BLACK);
        set_pixel(320 - i, 335 + i / 10, _BLACK);
    }

    return 0;
}