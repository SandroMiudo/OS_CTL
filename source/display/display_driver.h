#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <stdint.h>
#include "display_api.h"

typedef struct DisplayDriver {
    uint8_t (*init_routine)(void);
    void (*run)(void);
    // Drawing functions
    void (*draw_image)(unsigned int x, unsigned int y, unsigned int width, 
        unsigned int height, const uint32_t* buffer);
    void (*draw_pixel)(unsigned int x, unsigned int y, uint32_t color);
    void (*clear_screen)(uint8_t on);
    void (*fill_screen)(uint32_t color);

    // Query functions
    int (*get_width)(void);
    int (*get_height)(void);
} DisplayDriver;

// Global driver pointer
extern DisplayDriver* global_driver;

#endif
