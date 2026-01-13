#include "display_driver.h"

#if defined(DISPLAY_DRIVER_XLIB)

extern uint8_t xlib_init();
extern void xlib_run();
extern void xlib_draw_image(int x, int y, int width, int height, const uint32_t* buffer);
extern void xlib_draw_pixel(int x, int y, uint32_t color);

static DisplayDriver xlib_driver = {
    .init_routine = xlib_init,
    .run = xlib_run,
    .draw_image = xlib_draw_image,
    .draw_pixel = xlib_draw_pixel
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
