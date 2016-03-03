#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::DestinationOptions> destinationOptions;
    CRITICAL_SECTION cs;
} DestinationOptions;

extern PyTypeObject DestinationOptionsType;

DestinationOptions* DestinationOptions_New(std::shared_ptr<MI::DestinationOptions> destinationOptions);

