#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    MI::Application* app;
} Application;

extern PyTypeObject ApplicationType;
