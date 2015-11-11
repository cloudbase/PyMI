#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    MI::Instance* instance;
} Instance;

extern PyTypeObject InstanceType;

PyObject* Instance_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
