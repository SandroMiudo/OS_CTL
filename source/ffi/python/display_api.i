%module display_api

%{
#include "display_api.h"
#include <numpy/arrayobject.h>
%}

%include "stdint.i"
%include "display_api.h"

%init %{
import_array();
%}

%typemap(memberin) const uint32_t* cmd_draw_image_t::buffer {
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