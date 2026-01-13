#ifndef DISPLAY_DRIVER_DEFS
#define DISPLAY_DRIVER_DEFS

#include <stdint.h>

typedef uint32_t CONTROLLER_ID;
typedef uint32_t CONNECTOR_ID;
typedef uint32_t ENCODER_ID;

typedef struct _ConnMap {
    uint32_t type;
    const char* name;
} ConnMap, *ConnMapPtr;

#define CREQ_ARGB(_width, _height) \
    struct drm_mode_create_dumb creq = {0}; \
    creq.width  = _width; \
    creq.height = _height; \
    creq.bpp    = 32

#define CMD_ARGB(_creq) \
    struct drm_mode_fb_cmd2 cmd = {0}; \
    cmd.width    = _creq.width; \
    cmd.height   = _creq.height; \
    cmd.pixel_format = DRM_FORMAT_XRGB8888; \
    cmd.handles[0] = _creq.handle; \
    cmd.pitches[0] = _creq.pitch; \

#define MAX_CARD_PATH 64

#endif