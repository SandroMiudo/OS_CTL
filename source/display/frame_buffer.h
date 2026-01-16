#ifndef FRAME_BUFFER
#define FRAME_BUFFER
#include <stdint.h>

typedef struct _fb_info {
    uint32_t width;
    uint32_t height;
    uint32_t stride;    // bytes per row
    uint32_t bpp;       // bytes per pixel (4 for ARGB)
    uint8_t *buffer;    // mmap'd pointer
} fb_info, *fb_info_ptr;

typedef enum _FrameBufferFillCommands {
    WHITE_OUT,
    BLACK_OUT,
} FrameBufferFillCommands;

extern void fb_draw_image(fb_info *fb, int x0, int y0,
                   uint32_t img_width, uint32_t img_height,
                   const uint32_t *image);

extern void fb_set_pixel(fb_info *fb, int x, int y, uint32_t color);          

extern void fb_fill(fb_info_ptr fb, uint32_t to);

extern void fb_clear(fb_info_ptr fb, uint8_t on);

#define RGB_BPP 3
#define ARGB_BPP RGB_BPP + 1

#define FRAME_BUFFER_INFO(_width, _height, _stride, _bpp) \
    (fb_info){ .width = (_width), .height = (_height), .stride = (_stride), .bpp = (_bpp), .buffer = (NULL) }

    // _depth & _pad are in bits per pixel
#define FRAME_BUFFER_INFO_W_PAD(_name, _width, _height, _depth, _pad) \
    int _bpp = (int)(((_depth + _pad )) / 8); \
    fb_info _name = FRAME_BUFFER_INFO(_width, _height, _width * _bpp, _bpp)

#define FRAME_BUFFER_INFO_FROM_DEPTH(_name, _width, _height, _depth) \
    FRAME_BUFFER_INFO_W_PAD(_name, _width, _height, _depth, 0)

#define FRAME_BUFFER_INFO_RGB(_name, _width, _height) \
    FRAME_BUFFER_INFO(_width, _height, _width * RGB_BPP, RGB_BPP)

#define FRAME_BUFFER_INFO_ARGB(_name, _width, _height) \
    FRAME_BUFFER_INFO(_width, _height, _width * ARGB_BPP, ARGB_BPP)

#define SCREEN_INFO 0x0
#define WIDTH_INFO 0x1
#define HEIGHT_INFO 0x2
#define DEPTH_INFO 0x3

#define SPLIT_INFO_DATA(type, arr, a, b, c, d) \
    type a = (type)((arr)[SCREEN_INFO]), \
         b = (type)((arr)[WIDTH_INFO]),  \
         c = (type)((arr)[HEIGHT_INFO]), \
         d = (type)((arr)[DEPTH_INFO]);

#endif