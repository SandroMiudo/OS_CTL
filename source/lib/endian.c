#include "endian.h"

uint32_t endian_native(uint32_t v) { return v; }

uint32_t endian_swap(uint32_t v) {
    return ((v >> 24) & 0x000000FF) |
           ((v >>  8) & 0x0000FF00) |
           ((v <<  8) & 0x00FF0000) |
           ((v << 24) & 0xFF000000);
}

uint16_t endian_swap16(uint16_t v) {
    return (uint16_t)((v >> 8) | (v << 8));
}

uint32_t to_little_endian(uint32_t v) {
    return endian_swap(v);
}

uint16_t to_little_endian16(uint16_t v) {
    return endian_swap16(v);
}