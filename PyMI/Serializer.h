#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::Serializer> serializer;
    CRITICAL_SECTION cs;
} Serializer;

extern PyTypeObject SerializerType;

Serializer* Serializer_New(std::shared_ptr<MI::Serializer> serializer);
