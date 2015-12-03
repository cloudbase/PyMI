#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::Operation> operation;
} Operation;

extern PyTypeObject OperationType;

Operation* Operation_New(std::shared_ptr<MI::Operation> operation);
