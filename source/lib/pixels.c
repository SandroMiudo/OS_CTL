#include "pixels.h"

static const channel_indices_t CHANNEL_INDEX_TABLE[] = {
    [ORDER_RGBA] = { .r = 0, .g = 1, .b = 2, .a = 3 },
    [ORDER_ARGB] = { .r = 1, .g = 2, .b = 3, .a = 0 },
    [ORDER_BGRA] = { .r = 2, .g = 1, .b = 0, .a = 3 },
    [ORDER_ABGR] = { .r = 3, .g = 2, .b = 1, .a = 0 },
};

const channel_indices_t* get_channel_indices(channel_order_t order) {
    /* Optional safety check */
    if ((unsigned)order >= (unsigned)(sizeof(CHANNEL_INDEX_TABLE) /
                                      sizeof(CHANNEL_INDEX_TABLE[0]))) {
        return &CHANNEL_INDEX_TABLE[ORDER_RGBA]; // sane default
    }

    return &CHANNEL_INDEX_TABLE[order];
}
