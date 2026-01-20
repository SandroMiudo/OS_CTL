#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>

typedef enum {
    ENDIAN_LITTLE,
    ENDIAN_BIG
} endian_t;

#define HOST_ENDIAN ENDIAN_LITTLE
#define NET_ENDIAN ENDIAN_BIG

uint32_t endian_native(uint32_t v);
uint32_t endian_swap(uint32_t v);
uint32_t to_little_endian(uint32_t v);
uint16_t to_little_endian16(uint16_t v);

#endif
