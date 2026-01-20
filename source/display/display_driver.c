#include "display_driver.h"

#if defined(DISPLAY_DRIVER_XLIB)

extern uint8_t xlib_init();
extern void xlib_run();
extern void xlib_draw_image(unsigned int x, unsigned int y, unsigned int width, 
    unsigned int height, const uint32_t* buffer);
extern void xlib_draw_pixel(unsigned int x, unsigned int y, uint32_t color);
extern void xlib_clear_screen(uint8_t on);
extern void xlib_fill_screen(uint32_t color);

extern int xlib_get_width();
extern int xlib_get_height();

static DisplayDriver xlib_driver = {
    .init_routine = xlib_init,
    .run = xlib_run,
    .draw_image = xlib_draw_image,
    .draw_pixel = xlib_draw_pixel,
    .clear_screen = xlib_clear_screen,
    .fill_screen = xlib_fill_screen,
    \
    .get_width = xlib_get_width,
    .get_height = xlib_get_height
};

DisplayDriver* global_driver = &xlib_driver;

#elif defined(DISPLAY_DRIVER_DRM)

extern uint8_t drm_driver_init();
extern void drm_run();

static DisplayDriver drm_driver = {
    .init_routine = drm_driver_init,
    .run = drm_run
};

DisplayDriver* global_driver = &drm_driver;

#else
DisplayDriver* global_driver = (void*)0;
#endif
