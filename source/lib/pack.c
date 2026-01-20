#include "pack.h"
#include <stddef.h>

static uint32_t pack_rgba_le(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Memory layout: R G B A (little endian)
    return (a << 24) | (b << 16) | (g << 8) | r;
}

static uint32_t pack_argb_le(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Memory layout: A R G B (little endian)
    return (b << 24) | (g << 16) | (r << 8) | a;
}

static uint32_t pack_bgra_le(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Memory layout: B G R A (little endian)
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static uint32_t pack_abgr_le(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Memory layout: A B G R (little endian)
    return (r << 24) | (g << 16) | (b << 8) | a;
}

static const pack_fn_table_t PACK_FN_TABLE = {
    .rgba = pack_rgba_le,
    .argb = pack_argb_le,
    .bgra = pack_bgra_le,
    .abgr = pack_abgr_le
};

static const pack_ops_t PACK_OPS_TABLE[] = {
    [ORDER_RGBA] = { ORDER_RGBA, ENDIAN_LITTLE, PACK_FN_TABLE.rgba },
    [ORDER_ARGB] = { ORDER_ARGB, ENDIAN_LITTLE, PACK_FN_TABLE.argb },
    [ORDER_BGRA] = { ORDER_BGRA, ENDIAN_LITTLE, PACK_FN_TABLE.bgra },
    [ORDER_ABGR] = { ORDER_ABGR, ENDIAN_LITTLE, PACK_FN_TABLE.abgr }
};

const pack_ops_t* PACK_OPS_RGBA_LE = &PACK_OPS_TABLE[ORDER_RGBA];
const pack_ops_t* PACK_OPS_ARGB_LE = &PACK_OPS_TABLE[ORDER_ARGB];
const pack_ops_t* PACK_OPS_BGRA_LE = &PACK_OPS_TABLE[ORDER_BGRA];
const pack_ops_t* PACK_OPS_ABGR_LE = &PACK_OPS_TABLE[ORDER_ABGR];

// -------------------------
// Generic packer
// -------------------------

void pack_pixels(
    const uint8_t *src,
    uint32_t *dst,
    size_t pixels,
    const pack_ops_t *ops) {
    const channel_indices_t* indices = get_channel_indices(ops->order);

    for (size_t i = 0; i < pixels; i++) {
        uint8_t r = src[i * 4 + indices->r];
        uint8_t g = src[i * 4 + indices->g];
        uint8_t b = src[i * 4 + indices->b];
        uint8_t a = src[i * 4 + indices->a];

        dst[i] = ops->pack(r, g, b, a);
    }
}

const pack_ops_t* pack_ops_by_idx(channel_order_t order) {
    return &PACK_OPS_TABLE[order];
}

pack_ops_t make_pack_ops(channel_order_t order, endian_t endian) {
    pack_ops_t ops;
    ops.order = order;
    ops.endian = endian;

    // Currently only little-endian is supported
    if (endian != ENDIAN_LITTLE) {
        ops.pack = NULL;
        return ops;
    }

    switch (order) {
        case ORDER_RGBA: ops.pack = pack_rgba_le; break;
        case ORDER_ARGB: ops.pack = pack_argb_le; break;
        case ORDER_BGRA: ops.pack = pack_bgra_le; break;
        case ORDER_ABGR: ops.pack = pack_abgr_le; break;
        default: ops.pack = NULL; break;
    }

    return ops;
}