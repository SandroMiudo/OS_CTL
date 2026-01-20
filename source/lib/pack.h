#ifndef PACK_H
#define PACK_H

#include <stdint.h>
#include "pack.h"
#include "endian.h"
#include "pixels.h"
#include <stddef.h>

// -------------------------
// Function type for packing
// -------------------------
typedef uint32_t (*pack_fn_t)(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

typedef struct {
    channel_order_t order;
    endian_t endian;
    pack_fn_t pack;
} pack_ops_t;

typedef struct {
    pack_fn_t rgba;
    pack_fn_t argb;
    pack_fn_t bgra;
    pack_fn_t abgr;
} pack_fn_table_t;

extern const pack_ops_t* PACK_OPS_RGBA_LE;
extern const pack_ops_t* PACK_OPS_ARGB_LE;
extern const pack_ops_t* PACK_OPS_BGRA_LE;
extern const pack_ops_t* PACK_OPS_ABGR_LE;

pack_ops_t make_pack_ops(channel_order_t order, endian_t endian);
void pack_pixels(const uint8_t *src, uint32_t *dst,
    size_t pixels, const pack_ops_t *ops);

const pack_ops_t* pack_ops_by_idx(channel_order_t order);

#endif