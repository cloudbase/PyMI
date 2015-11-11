#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    MI::Class* m_class;
} Class;

PyObject* Class_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

extern PyTypeObject ClassType;
