#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    PyObject* app;
    MI::Session* session;
} Session;

extern PyTypeObject SessionType;