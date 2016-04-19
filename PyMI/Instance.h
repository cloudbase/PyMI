#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::Instance> instance;
    CRITICAL_SECTION cs;
} Instance;

extern PyTypeObject InstanceType;

Instance* Instance_New(std::shared_ptr<MI::Instance> instance);
