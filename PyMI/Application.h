#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::Application> app;
    CRITICAL_SECTION cs;
} Application;

extern PyTypeObject ApplicationType;
