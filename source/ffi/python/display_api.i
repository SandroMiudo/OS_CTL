%module display_api

%{
#include "display_api.h"
#include <numpy/arrayobject.h>

extern void display_cb_verbose(char* msg, int status);
extern void display_cb_log_wo(char* _, int status);
extern void display_cb_log_u_only(char* msg, int _);
extern void display_cb_non(char* msg, int status);
%}

%include "stdint.i"
%include "display_api.h"

%init %{
import_array();
%}

void display_draw_image(const cmd_draw_image_t* cmd, display_flags_t flags, display_callback_f cb = display_cb_non, char* msg);
void display_set_pixel(const cmd_set_pixel_t* cmd, display_flags_t flags, display_callback_f cb = display_cb_non, char* msg);
void display_clear_screen(const cmd_clear_screen_t* cmd, display_flags_t flags, display_callback_f cb = display_cb_non, char* msg);
void display_fill_screen(const cmd_fill_screen_t* cmd, display_flags_t flags, display_callback_f cb = display_cb_non, char* msg);

// callback functions for display api
%constant void display_cb_verbose(char* msg, int status);
%constant void display_cb_log_wo(char* _, int status);
%constant void display_cb_log_u_only(char* msg, int _);
%constant void display_cb_non(char* msg, int status);

%typemap(memberin) uint32_t* cmd_draw_image_t::buffer {
    // Convert Python object to a contiguous NumPy array of uint32
    PyArrayObject *arr = (PyArrayObject*) PyArray_FROM_OTF(
        $input,              // Python object assigned to the field
        NPY_UINT32,          // Must be dtype uint32
        NPY_ARRAY_IN_ARRAY   // Must be C-contiguous & aligned
    );
    if (!arr) {
        SWIG_exception_fail(SWIG_TypeError,
                            "Expected a NumPy array of dtype uint32");
    }

    // Assign raw pointer to the struct field
    $1 = (const uint32_t*) PyArray_DATA(arr);

    // Keep array alive to prevent Python GC from freeing the memory
    Py_INCREF(arr);
    SWIG_Python_AppendOutput($input,
        SWIG_NewPointerObj((void*)arr, SWIGTYPE_p_PyArrayObject, 0));
}