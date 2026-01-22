%module display_api

%{
#include "display_api.h"
#include "numpy_api.h"
#include "endian.h"
#include "pixels.h"
#include <Python.h>

extern void display_cb_verbose(char* msg, int status);
extern void display_cb_log_wo(char* _, int status);
extern void display_cb_log_u_only(char* msg, int _);
extern void display_cb_non(char* msg, int status);

extern void cmd_draw_image_set_buffer_u8(cmd_draw_image_t *cmd, PyObject *obj);
extern void cmd_draw_image_set_buffer_u32(cmd_draw_image_t *cmd, PyObject *obj, endian_t endianness);
%}

%ignore cmd_draw_image_t::buffer; // custom handling
%ignore endian_native;
%ignore endian_swap;
%ignore to_little_endian;
%ignore to_little_endian16;
%ignore channel_indices_t;
%ignore get_channel_indices;

%include "stdint.i"
%include "display_api.h"
%include "endian.h"
%include "pixels.h"

%init %{
    import_array();
    if (PyArray_API == NULL) {
        PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
    }
%}

%extend cmd_draw_image_t {
    void set_buffer_u8(PyObject *obj) {
        cmd_draw_image_set_buffer_u8($self, obj);
    }

    void set_buffer_u32(PyObject *obj, endian_t endianness) {
        cmd_draw_image_set_buffer_u32($self, obj, endianness);
    }
}

// TODO: property binding %property(cmd_draw_image_t, buffer=set_buffer); 

// callback functions for display api
%constant void display_cb_verbose(char* msg, int status);
%constant void display_cb_log_wo(char* _, int status);
%constant void display_cb_log_u_only(char* msg, int _);
%constant void display_cb_non(char* msg, int status);