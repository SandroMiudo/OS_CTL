#ifndef DISPLAY_API_H
#define DISPLAY_API_H

#include <stdint.h>

typedef enum {
    DISPLAY_DRAW_IMAGE,
    DISPLAY_SET_PIXEL,
    DISPLAY_CLEAR_SCREEN,
    DISPLAY_QUERY_WIDTH,
    DISPLAY_QUERY_HEIGHT
} display_command_t;

// For DISPLAY_DRAW_IMAGE
typedef struct {
    int x;
    int y;
    int width;
    int height;
    const uint32_t* buffer;
} cmd_draw_image_t;

// For DISPLAY_SET_PIXEL
typedef struct {
    int x;
    int y;
    uint32_t color;
} cmd_set_pixel_t;

// For CLEAR_SCREEN
typedef struct {
    uint32_t color;
} cmd_clear_screen_t;

// --------------------
// Dedicated functions
// --------------------

void display_draw_image(const cmd_draw_image_t* cmd);
void display_set_pixel(const cmd_set_pixel_t* cmd);
void display_clear_screen(const cmd_clear_screen_t* cmd);

int display_query_width(void);
int display_query_height(void);

#endif // DISPLAY_API_H
