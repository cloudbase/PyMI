#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::Instance> instance;
} Instance;

extern PyTypeObject InstanceType;

Instance* Instance_New(std::shared_ptr<MI::Instance> instance);
