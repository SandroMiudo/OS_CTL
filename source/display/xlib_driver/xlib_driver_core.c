// xlib_driver_core.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <X11/Xlib.h>
#include "display_driver.h"
#include "frame_buffer.h"
#include "global_structs.h"
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <string.h>
#include <stdlib.h>
#include <grp.h>
#include <sys/types.h>

static const char *const _shared_memory_fb_data = "/frame_buffer_data";
static const char *const _shared_memory_fb_info = "/frame_buffer_info";
static const char *const _group_name = "seat_display_driver"; // move it into display driver

extern int open_shm_w_group_mask(
    const char *shm_name, const char *group_name, int flags, mode_t mask);

void obtain_display_info(int (*p)[4]);

// Store Display* and Window globally for this driver module
static Display *driver_display = NULL;
static Window driver_window = 0;

/* Shared non-Xlib logging helper: prints error details without calling Xlib. */
static void xlib_log_error_details(const char *tag,
                                  unsigned long serial,
                                  int error_code,
                                  int request_code,
                                  int minor_code,
                                  unsigned long resourceid,
                                  const char *description) {
    fprintf(stderr, "%s serial=%lu error_code=%d request_code=%d minor_code=%d resource=0x%lx\n",
            tag, serial, error_code, request_code, minor_code, resourceid);
    if (description && description[0]) {
        fprintf(stderr, "%s description: %s\n", tag, description);
    }
}

/* X error handler: prints detailed XErrorEvent info and human-readable text. */
static int xlib_x_error_handler(Display *dpy, XErrorEvent *ev) {
    char err_text[128] = {0};
    /* Try to get human-readable error text from the server */
    XGetErrorText(dpy, ev->error_code, err_text, sizeof(err_text));

    /* Use shared helper to print details */
    xlib_log_error_details("[XLIB ERROR]",
                           ev->serial,
                           ev->error_code,
                           ev->request_code,
                           ev->minor_code,
                           ev->resourceid,
                           err_text);

    /* Return 0 to let Xlib continue its default error processing where appropriate */
    return 0;
}

/* X I/O error handler: called on fatal I/O errors (connection lost).
 * Should not attempt Xlib calls; print an explanatory message and terminate.
 */
static int xlib_io_error_handler(Display *dpy) {
    int errnum = errno;
    /* IO handler must not call Xlib; use the shared logger with limited info */
    if (errnum != 0) {
        char desc[128];
        snprintf(desc, sizeof(desc), "errno=%d (%s)", errnum, strerror(errnum));
        xlib_log_error_details("[XLIB IO ERROR]", 0UL, errnum, 0, 0, 0UL, desc);
    } else {
        xlib_log_error_details("[XLIB IO ERROR]", 0UL, -1, 0, 0, 0UL, "connection lost");
    }
    _exit(1);
    return 0; /* unreachable */
}

/**
 * Xlib driver init routine.
 * @param display_id - The display string (e.g., ":1") passed by display_manager
 * @return 0 on success, non-zero on failure
 */
uint8_t xlib_init() {
    // Open the specified X display

    setvbuf(stdout, NULL, _IOLBF, 0);
    printf("Using XLIB Driver ...\n");

    driver_display = XOpenDisplay(NULL);
    if (!driver_display) {
        fprintf(stderr, "Failed to open X display:\n");
        return 1;
    }

    /* register X error handler to get readable X server errors in our logs */
    XSetErrorHandler(xlib_x_error_handler);
    /* register IO error handler: logs and terminates on fatal I/O errors */
    XSetIOErrorHandler(xlib_io_error_handler);
    /* do not force global synchronous mode (can cause transient EAGAIN IO errors)
     * Use XSync at points where we need to ensure errors are processed. */
    XSynchronize(driver_display, False);

    int screen = DefaultScreen(driver_display);
    Window root = RootWindow(driver_display, screen);

    int width = DisplayWidth(driver_display, screen);
    int height = DisplayHeight(driver_display, screen);
    printf("X display opened : %dx%d\n", width, height);

    // Create a window that covers the full screen
    driver_window = XCreateSimpleWindow(
        driver_display,
        root,
        0, 0,             // x, y position
        width, height,    // width, height (full screen)
        0,                // border width
        WhitePixel(driver_display, screen),
        BlackPixel(driver_display, screen)
    );

    // Map window and flush commands to the X server
    XMapWindow(driver_display, driver_window);
    XFlush(driver_display);

    printf("Flushed window == init completed ...\n");

    return 0; // Success
}

void dump_screen_context(int (*info_data_arr)[4]) {
    SPLIT_INFO_DATA(int, *info_data_arr, s, w, h, d);

    printf("Screen Context:\n\tScreen = %d\n\tWidth = %d\n\tHeight = %d\n\tDepth = %d\n",
        s, w, h, d);
}

void dump_frame_buffer_context(fb_info_ptr fb) {
    if (!fb) {
        printf("Frame Buffer context: (null)\n");
        return;
    }

    size_t expected_size = (size_t)fb->stride * (size_t)fb->height;

    printf("Frame Buffer context:\n");
    printf("\tWidth  = %u\n", fb->width);
    printf("\tHeight = %u\n", fb->height);
    printf("\tStride = %u (bytes per row)\n", fb->stride);
    printf("\tBPP    = %u (bytes per pixel)\n", fb->bpp);
    printf("\tBuffer = %.64s\n", fb->name);
    printf("\tExpected allocation size = %zu bytes\n", expected_size);
}

void obtain_display_info(int (*info_data_arr)[4]) {
    int screen = DefaultScreen(driver_display);
    int width = DisplayWidth(driver_display, screen);
    int height = DisplayHeight(driver_display, screen);
    int depth = DefaultDepth(driver_display, screen);

    (*info_data_arr)[SCREEN_INFO] = screen;
    (*info_data_arr)[WIDTH_INFO] = width;
    (*info_data_arr)[HEIGHT_INFO] = height;
    (*info_data_arr)[DEPTH_INFO] = depth;
}

int xlib_run() {
    int fd;
    if (!driver_display || !driver_window) {
        fprintf(stderr, "[xlib_run] ERROR: X display or window not initialized.\n");
        return 1;
    }

    int info_data_arr[4];
    printf("[xlib_run] calling obtain_display_info()...\n");
    obtain_display_info(&info_data_arr);
    printf("[xlib_run] obtain_display_info() returned: screen=%d width=%d height=%d depth=%d\n",
           info_data_arr[SCREEN_INFO], info_data_arr[WIDTH_INFO], info_data_arr[HEIGHT_INFO], info_data_arr[DEPTH_INFO]);

    SPLIT_INFO_DATA(int, info_data_arr, s, w, h, d);

    printf("[xlib_run] FRAME_BUFFER_INFO_FROM_DEPTH with w=%d h=%d d=%d\n", w, h, d);
    FRAME_BUFFER_INFO_W_PAD(_global_fb, w, h, d, 8);

    size_t fb_size = (size_t)_global_fb.stride * (size_t)_global_fb.height;
    printf("[xlib_run] allocating framebuffer: stride=%d height=%d size=%zu\n", _global_fb.stride, _global_fb.height, fb_size);

    // unlink shm objects if they still exist
    // TODO: write systemd on signal handler
    shm_unlink(_shared_memory_fb_data);
    shm_unlink(_shared_memory_fb_info);

    if ((fd = open_shm_w_group_mask(_shared_memory_fb_data,
        _group_name, O_CREAT | O_RDWR, S_IRWXO | S_IXGRP | S_IXUSR)) == -1) {
        return 1;
    }

    if (ftruncate(fd, fb_size) != 0) {
        perror("error during ftruncate");
        return 1;
    }

    frame_buffer_data fb_data_sync = (frame_buffer_data)mmap(NULL, fb_size, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_POPULATE, fd, 0);

    if (fb_data_sync == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // keep buffer pinned
    if (mlock(fb_data_sync, sizeof(fb_info))) {
        perror("error during mlock");
        return 1;
    }

    memset(fb_data_sync, '\0', fb_size);

    strncpy(_global_fb.name, _shared_memory_fb_data, sizeof(_global_fb.name)-1);
    _global_fb.name[sizeof(_global_fb.name)-1] = '\0';
    _global_fb.b_size = fb_size;

    if ((fd = open_shm_w_group_mask(_shared_memory_fb_info,
        _group_name, O_CREAT | O_RDWR, S_IRWXO | S_IXGRP | S_IXUSR)) == -1) {
        return 1;
    }

    if (ftruncate(fd, sizeof(fb_info)) != 0) {
        perror("error during ftruncate");
        return 1;
    }

    fb_info_ptr fb_info_sync = (fb_info_ptr)mmap(NULL, sizeof(fb_info), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);

    *fb_info_sync = _global_fb;

    dump_screen_context(&info_data_arr);
    dump_frame_buffer_context(fb_info_sync);

    LOG("[xlib_run] creating XImage (w=%d h=%d depth=%d)...\n", w, h, d);
    XImage* driver_ximage = XCreateImage(
        driver_display,
        DefaultVisual(driver_display, s),
        d,
        ZPixmap,
        0,
        (char *)fb_data_sync,
        w,
        h,
        32,
        _global_fb.stride
    );

    // unmap fb_info after writing data to it
    munmap(fb_info_sync, sizeof(fb_info));

    if (!driver_ximage) {
        fprintf(stderr, "[xlib_run] ERROR: Failed to create XImage.\n");
        return 1;
    }
    printf("[xlib_run] XImage created successfully. bytes_per_line=%d bytes_per_pixel=%d\n",
           driver_ximage->bytes_per_line, driver_ximage->bits_per_pixel/8);

    LOG("[xlib_run] obtaining GC...\n");
    GC gc = DefaultGC(driver_display, s);
    if (gc == NULL)
        fprintf(stderr, "[xlib_run] WARNING: DefaultGC returned NULL.\n");
    else
        printf("[xlib_run] DefaultGC acquired.\n");
    
    // Target FPS
    const int target_fps = 60;
    const int frame_delay_us = 1000000 / target_fps;

    LOG("[xlib_run] entering render loop (target %d FPS).\n", target_fps);
    unsigned long frame_counter = 0;

    while (1) {
        // Update the window with the current framebuffer
        XPutImage(driver_display, driver_window, gc, driver_ximage,
                  0, 0, // src
                  0, 0, // dst
                  w, h);

        // Flush the X output buffer and synchronously process errors/events
        XFlush(driver_display);
        // Process any pending errors/events now
        // XSync(driver_display, False);

        // Log every 300 frames to avoid spamming logs
        if ((frame_counter++ % 300) == 0) {
            LOG("[xlib_run] frame %lu updated (w=%d h=%d).\n", frame_counter, w, h);
        }

        // Sleep to maintain ~target FPS
        if (usleep(frame_delay_us) != 0) {
            fprintf(stderr, "[xlib_run] WARNING: usleep interrupted.\n");
        }
    }

    // Cleanup (not reached in infinite loop)
    printf("[xlib_run] cleaning up XImage and framebuffer.\n");
    XDestroyImage(driver_ximage);

    // normally its enough to just unmap the virtual pages, and this would
    // automatically skip the unlocking step
    munlock(fb_data_sync, fb_size);
    munmap(fb_data_sync, fb_size);

    /* Ensure server processed all requests and close connection cleanly */
    XSync(driver_display, False);
    XCloseDisplay(driver_display);

    return 0;
}

/**
 * Optional getter functions for other modules to access the Display* and Window
 */
Display* get_driver_display(void) {
    return driver_display;
}

Window get_driver_window(void) {
    return driver_window;
}

static int __get_shm_objects(fb_info_ptr* _fb_info, frame_buffer_data* fb_data) {
    int fd;
    if ((fd = shm_open(_shared_memory_fb_info, O_RDWR, 0)) == -1) {
        perror("error during shm_open");
        return 1;
    }
    fb_info_ptr fb_info_sync = (fb_info_ptr)mmap(NULL, sizeof(fb_info), PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);

    close(fd);

    if ((fd = shm_open(_shared_memory_fb_data, O_RDWR, 0)) == -1) {
        perror("error during shm_open");
        return 1;
    }

    frame_buffer_data fb_data_sync = (frame_buffer_data)mmap(NULL, fb_info_sync->b_size, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_POPULATE, fd, 0);

    close(fd);

    *_fb_info = fb_info_sync;
    *fb_data = fb_data_sync;

    return 0;
}

static void __release_shm_objects(fb_info_ptr fb_info, frame_buffer_data fb_data) {
    munmap(fb_data, fb_info->b_size);
    munmap(fb_info, sizeof(fb_info));
}

void xlib_draw_image(unsigned int x, unsigned int y, 
    unsigned int width, unsigned int height, 
    const uint32_t* buffer) {
    fb_info_ptr fb_info;
    frame_buffer_data fb_data;

    if (__get_shm_objects(&fb_info, &fb_data))
        return

    LOG("[xlib_driver_draw_image]\n");

    fb_draw_image(fb_info, fb_data, x, y, width, height, buffer);

    __release_shm_objects(fb_info, fb_data);
}

void xlib_draw_pixel(unsigned int x, unsigned int y, uint32_t color) {
    fb_info_ptr fb_info;
    frame_buffer_data fb_data;

    if (__get_shm_objects(&fb_info, &fb_data))
        return

    LOG("[xlib_driver_draw_pixel]\n");

    fb_set_pixel(fb_info, fb_data, x, y, color);

    __release_shm_objects(fb_info, fb_data);
}

void xlib_clear_screen(uint8_t on) {
    fb_info_ptr fb_info;
    frame_buffer_data fb_data;

    if (__get_shm_objects(&fb_info, &fb_data))
        return

    LOG("[xlib_driver_clear_screen]\n");

    fb_clear(fb_info, fb_data, on);

    __release_shm_objects(fb_info, fb_data);
}

void xlib_fill_screen(uint32_t color) {
    fb_info_ptr fb_info;
    frame_buffer_data fb_data;

    if (__get_shm_objects(&fb_info, &fb_data))
        return

    LOG("[xlib_driver_fill_screen]\n");

    fb_fill(fb_info, fb_data, color);

    __release_shm_objects(fb_info, fb_data);
}

Display* xlib_resolution_prop_query() {
    char* d_no = getenv("DISPLAY");
    char buf[64];

    if (!d_no) {
        fprintf(stderr, "Failed to open X display - DISPLAY not specified :\n");
        return NULL;
    }

    // auto prefix : if not specified
    if (memchr(d_no, ':', 1) == NULL){
        snprintf(buf, 64, ":%s", getenv("DISPLAY"));
        setenv("DISPLAY", buf, 1);
    }

    Display* driver_d = XOpenDisplay(NULL);
    if (!driver_d) {
        fprintf(stderr, "Failed to open X display:\n");
        return NULL;
    }

    return driver_d;
}

// import this function relies on passing in the correct display using the DISPLAY env variable!
int xlib_get_width() {
    Display* d = xlib_resolution_prop_query();

    LOG("obtained display = %p\n", d);

    if (d == NULL)
        return 1;

    return DisplayWidth(d, DefaultScreen(d));
}

int xlib_get_height() {
    Display* d = xlib_resolution_prop_query();

    LOG("obtained display = %p\n", d);

    if (d == NULL)
        return 1;

    return DisplayHeight(d, DefaultScreen(d));
}