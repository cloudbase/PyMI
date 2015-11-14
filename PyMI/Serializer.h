#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        MI::Serializer* serializer;
} Serializer;

extern PyTypeObject SerializerType;

Serializer* Serializer_New(MI::Serializer* serializer);
