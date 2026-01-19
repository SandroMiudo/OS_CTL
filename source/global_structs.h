#ifndef GLOBAL_STRUCTS
#define GLOBAL_STRUCTS

#include <stdint.h>
#include <semaphore.h>
#include <stdarg.h>

#define DEBUG

typedef struct seat_shared_ds {
    sem_t sync_state;
} seat_shared_ds, *seat_shared_ds_ptr;

#ifdef DEBUG
    #define LOG(fmt, ...) log_printf("[%s:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define LOG(fmt, ...) 
#endif

static inline void log_printf(char* fmt, ...) {
    #ifdef DEBUG
    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);

    va_end(ap);
    #endif
}

#endif