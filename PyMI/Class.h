#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    MI::Class* miClass;
} Class;

extern PyTypeObject ClassType;

Class* Class_New(MI::Class* miClass);
