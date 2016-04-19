#pragma once

#include <Python.h>
#include <MI++.h>
#include <vector>
#include <memory>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    std::shared_ptr<MI::Session> session;
    std::shared_ptr<std::vector<std::shared_ptr<MI::Callbacks>>> operationCallbacks;
    CRITICAL_SECTION cs;
} Session;

extern PyTypeObject SessionType;

Session* Session_New(std::shared_ptr<MI::Session> session);
