#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::OperationOptions> operationOptions;
} OperationOptions;

extern PyTypeObject OperationOptionsType;

OperationOptions* OperationOptions_New(std::shared_ptr<MI::OperationOptions> operationOptions);

