#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    MI::Instance* instance;
    bool ownsInstance;
} Instance;

extern PyTypeObject InstanceType;

Instance* Instance_New(MI::Instance* instance, bool ownsInstance=true);
