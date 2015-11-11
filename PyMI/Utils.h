#pragma once

#include <Python.h>
#include <mi.h>

PyObject* MI2Py(const MI_Value& value, MI_Type valueType, MI_Uint32 flags);
void Py2MI(PyObject* pyValue, MI_Value& value, MI_Type valueType);
