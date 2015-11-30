#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        MI::OperationOptions* operationOptions;
} OperationOptions;

extern PyTypeObject OperationOptionsType;

OperationOptions* OperationOptions_New(MI::OperationOptions* operationOptions);

