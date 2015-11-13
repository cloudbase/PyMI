#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    MI::Session* session;
} Session;

extern PyTypeObject SessionType;

Session* Session_New(MI::Session* session);
