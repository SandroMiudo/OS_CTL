#include "display_driver.h"
#include "display_api.h"
#include "frame_buffer.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h> 
#include "global_structs.h"
#include "numpy_api.h"
#include "pack.h"
#include "endian.h"
#include <stdlib.h>
#include <malloc.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* logs message only */
static void log_message(const char *msg){
    if (msg)
        fprintf(stderr, "%s\n", msg);
}

/* logs message + pthread error */
static void log_message_with_status(const char *msg, int status) {
    if (msg)
        fprintf(stderr, "%s: %s\n", msg, strerror(status));
    else
        fprintf(stderr, "%s\n", strerror(status));
}

// logs both
void display_cb_verbose(char* msg, int status) {
    log_message_with_status(msg, status);
}

// logs only per function error message
void display_cb_log_wo(char* _, int status) {
    log_message_with_status(NULL, status);
}

// logs only message
void display_cb_log_u_only(char* msg, int _) {
    log_message(msg);
}

// logs non 
void display_cb_non(char* msg, int status) {}

void display_draw_image(cmd_draw_image_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg){
    int status;
    LOG("[display_draw_image]\n");
    if (global_driver && global_driver->draw_image) {
        status = (flags & DISPLAY_WNO) ? 
            pthread_mutex_trylock(&lock) : 
            pthread_mutex_lock(&lock);
        if(status) {
            cb(msg, status);
            return;
        }
        global_driver->draw_image(
            cmd->x, cmd->y,
            cmd->width, cmd->height,
            cmd->buffer
        );
        pthread_mutex_unlock(&lock);
    }
}

void display_set_pixel(cmd_set_pixel_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg){
    int status;
    LOG("[display_set_pixel]\n");
    if (global_driver && global_driver->draw_pixel) {
        status = (flags & DISPLAY_WNO) ? 
            pthread_mutex_trylock(&lock) : 
            pthread_mutex_lock(&lock);
        if(status) {
            cb(msg, status);
            return;
        }
        global_driver->draw_pixel(cmd->x, cmd->y, cmd->color);
        pthread_mutex_unlock(&lock);
    }       
}

void display_clear_screen(cmd_clear_screen_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg){
    int status;
    LOG("[display_clear_screen]\n");
    if (global_driver && global_driver->clear_screen) {
        status = (flags & DISPLAY_WNO) ? 
            pthread_mutex_trylock(&lock) : 
            pthread_mutex_lock(&lock);
        if(status) {
            cb(msg, status);
            return;
        }
        global_driver->clear_screen(cmd->on);
        pthread_mutex_unlock(&lock);
    }
}

void display_fill_screen(cmd_fill_screen_t* cmd, display_flags_t flags, 
    display_callback_f cb, char* msg) {
    int status;
    LOG("[display_fill_screen]\n");
    if (global_driver && global_driver->fill_screen) {
        status = (flags & DISPLAY_WNO) ? 
            pthread_mutex_trylock(&lock) : 
            pthread_mutex_lock(&lock);
        if(status) {
            cb(msg, status);
            return;
        }
        global_driver->fill_screen(cmd->color);
        pthread_mutex_unlock(&lock);
    }
}

int display_query_width() {
    LOG("[display_query_width]\n");
    if (global_driver && global_driver->get_width)
        return global_driver->get_width();

    return -1;
}

int display_query_height() {
    LOG("[display_query_height]\n");
    if (global_driver && global_driver->get_height)
        return global_driver->get_height();

    return -1;
}

void cmd_draw_image_set_buffer_u8(cmd_draw_image_t *cmd, PyObject *obj) {
    assert(PyArray_API != NULL);

    PyArrayObject *arr = (PyArrayObject*)PyArray_FROM_OTF(
        obj,
        NPY_UINT8,
        NPY_ARRAY_IN_ARRAY | NPY_ARRAY_ENSUREARRAY
    );

    if (!arr) {
        PyErr_SetString(PyExc_TypeError, "Expected a numpy uint8 array");
        PyErr_Print();
        return;
    }

    // -------------------------
    // Validate shape and type
    // -------------------------
    int ndim = PyArray_NDIM(arr);
    if (ndim != 3) {
        PyErr_SetString(PyExc_ValueError, "Expected a 3D numpy array (H,W,4)");
        Py_DECREF(arr);
        return;
    }

    npy_intp *shape = PyArray_SHAPE(arr);
    if (shape[2] != 4) {
        PyErr_SetString(PyExc_ValueError, "Expected 4 channels (RGBA)");
        Py_DECREF(arr);
        return;
    }

    if (PyArray_ITEMSIZE(arr) != 1) {
        PyErr_SetString(PyExc_ValueError, "Expected uint8 array");
        Py_DECREF(arr);
        return;
    }

    // -------------------------
    // Allocate aligned buffer
    // -------------------------
    npy_intp h = shape[0];
    npy_intp w = shape[1];
    size_t pixels = (size_t)h * (size_t)w;

    uint32_t *buffer = memalign(sizeof(uint32_t), pixels * sizeof(uint32_t));

    if (buffer == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate aligned buffer");
        Py_DECREF(arr);
        return;
    }

    // -------------------------
    // Copy pixels (pack order)
    // -------------------------
    uint8_t *src = (uint8_t*)PyArray_DATA(arr);

    const pack_ops_t* p_ops = pack_ops_by_idx(cmd->order);

    pack_pixels(src, buffer, pixels, p_ops);

    cmd->buffer = buffer;

    // we don't need to hold the ref anymore, since we now handle it 
    // on c side
    Py_DECREF(arr);
}

void cmd_draw_image_set_buffer_u32(cmd_draw_image_t *cmd, 
    PyObject *obj, endian_t endianness) {
    assert(PyArray_API != NULL);

    PyArrayObject *arr = (PyArrayObject*)PyArray_FROM_OTF(
        obj,
        NPY_UINT32,
        NPY_ARRAY_IN_ARRAY | NPY_ARRAY_ENSUREARRAY
    );

    if (!arr) {
        PyErr_SetString(PyExc_TypeError, "Expected a numpy uint32 array");
        PyErr_Print();
        return;
    }

    // -------------------------
    // Validate shape and type
    // -------------------------
    int ndim = PyArray_NDIM(arr);
    if (ndim != 2) {
        PyErr_SetString(PyExc_ValueError, "Expected a 2D numpy array (H,W)");
        Py_DECREF(arr);
        return;
    }

    if (PyArray_ITEMSIZE(arr) != 4) {
        PyErr_SetString(PyExc_ValueError, "Expected uint32 array");
        Py_DECREF(arr);
        return;
    }

    npy_intp *shape = PyArray_SHAPE(arr);
    npy_intp h = shape[0];
    npy_intp w = shape[1];
    size_t pixels = (size_t)h * (size_t)w;

    // -------------------------
    // Allocate aligned buffer
    // -------------------------
    uint32_t *buffer = memalign(sizeof(uint32_t), pixels * sizeof(uint32_t));
    if (buffer == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate aligned buffer");
        Py_DECREF(arr);
        return;
    }

    // -------------------------
    // Convert endianness to little endian : TODO : make it arch dependent !
    // -------------------------
    uint32_t *src = (uint32_t*)PyArray_DATA(arr);

    if (endianness != HOST_ENDIAN) {
        for (size_t i = 0; i < pixels; i++)
            buffer[i] = to_little_endian(src[i]);
    }

    cmd->buffer = buffer;

    Py_DECREF(arr);
}