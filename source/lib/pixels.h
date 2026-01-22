#ifndef PIXEL_H
#define PIXEL_H

#include <stdint.h>

typedef enum {
    ORDER_RGBA,
    ORDER_ARGB,
    ORDER_BGRA,
    ORDER_ABGR
} channel_order_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} channel_indices_t;

const channel_indices_t* get_channel_indices(channel_order_t order);

#endif