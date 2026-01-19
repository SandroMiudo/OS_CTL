%module display_api

%{
#include "display_api.h"
#include "numpy_api.h"

extern void display_cb_verbose(char* msg, int status);
extern void display_cb_log_wo(char* _, int status);
extern void display_cb_log_u_only(char* msg, int _);
extern void display_cb_non(char* msg, int status);

extern void cmd_draw_image_set_buffer(cmd_draw_image_t *cmd, PyObject *obj);
%}

%ignore cmd_draw_image_t::buffer; // custom handling

%include "stdint.i"
%include "display_api.h"

%init %{
    import_array();
    if (PyArray_API == NULL) {
        PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
    }
%}

%extend cmd_draw_image_t {
    void set_buffer(PyObject *obj) {
        cmd_draw_image_set_buffer($self, obj);
    }
}

// TODO: property binding %property(cmd_draw_image_t, buffer=set_buffer); 

// callback functions for display api
%constant void display_cb_verbose(char* msg, int status);
%constant void display_cb_log_wo(char* _, int status);
%constant void display_cb_log_u_only(char* msg, int _);
%constant void display_cb_non(char* msg, int status);