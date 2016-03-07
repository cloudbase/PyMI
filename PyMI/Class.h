#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::Class> miClass;
    CRITICAL_SECTION cs;
} Class;

extern PyTypeObject ClassType;

Class* Class_New(std::shared_ptr<MI::Class> miClass);
