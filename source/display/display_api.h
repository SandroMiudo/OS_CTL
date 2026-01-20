#ifndef DISPLAY_API_H
#define DISPLAY_API_H

#include <stdint.h>
#include <Python.h>
#include "pixels.h"

#define WHITE 0 // off
#define BLACK 1 // on

typedef enum {
    DISPLAY_DRAW_IMAGE,
    DISPLAY_SET_PIXEL,
    DISPLAY_CLEAR_SCREEN,
    DISPLAY_QUERY_WIDTH,
    DISPLAY_QUERY_HEIGHT
} display_command_t;

typedef enum {
    DISPLAY_WNO = 1// non blocking
} display_flags_t;

// For DISPLAY_DRAW_IMAGE
typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    uint32_t* buffer;
    channel_order_t order;
} cmd_draw_image_t;

// For DISPLAY_SET_PIXEL
typedef struct {
    unsigned int x;
    unsigned int y;
    uint32_t color;
} cmd_set_pixel_t;

// For CLEAR_SCREEN
typedef struct {
    uint8_t on; // binary clear
} cmd_clear_screen_t;

// For FILL_SCREEN
typedef struct {
    uint32_t color;
} cmd_fill_screen_t;

// --------------------
// Dedicated functions
// --------------------

typedef void (*display_callback_f)(char* msg, int status);

void display_draw_image(cmd_draw_image_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg);
void display_set_pixel(cmd_set_pixel_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg);
void display_clear_screen(cmd_clear_screen_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg);
void display_fill_screen(cmd_fill_screen_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg);

// --------------------
// Query functions
// --------------------

int display_query_width();
int display_query_height();

#endif // DISPLAY_API_H
