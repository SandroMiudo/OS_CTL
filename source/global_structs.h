#ifndef GLOBAL_STRUCTS
#define GLOBAL_STRUCTS

#include <stdint.h>
#include <semaphore.h>

typedef struct seat_shared_ds {
    sem_t sync_state;
} seat_shared_ds, *seat_shared_ds_ptr;

#endif