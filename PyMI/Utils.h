#pragma once

#include <Python.h>
#include <mi.h>

PyObject* MI2Py(const MI_Value& value, MI_Type valueType);
