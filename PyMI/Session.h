#pragma once

#include <Python.h>
#include <MI++.h>
#include <vector>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    MI::Session* session;
    std::vector<MI::Callbacks*>* operationCallbacks;
} Session;

extern PyTypeObject SessionType;

Session* Session_New(MI::Session* session);
