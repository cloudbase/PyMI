#pragma once

#include <Python.h>
#include <MI++.h>

static PyMethodDef mi_error_methods[] = {
    { NULL, NULL, 0, NULL }  /* Sentinel */
};

#ifdef IS_PY3K
static PyModuleDef mi_error_module = {
    PyModuleDef_HEAD_INIT,
    "mi_error",
    "MI errors module.",
    -1,
    mi_error_methods
};
#endif

PyObject* MiError_Init(void);
