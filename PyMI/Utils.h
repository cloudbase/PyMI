#pragma once

#include <Python.h>
#include <mi.h>
#include <MIExceptions.h>
#include <string>

PyObject* MI2Py(const MI_Value& value, MI_Type valueType, MI_Uint32 flags);
void Py2MI(PyObject* pyValue, MI_Value& value, MI_Type valueType);
bool GetIndexOrName(PyObject *item, wchar_t* w, Py_ssize_t& i);
void SetPyException(const std::exception& ex);

class OutOfMemoryException : public MI::Exception
{
public:
    OutOfMemoryException() : MI::Exception(L"Out of memory") {}
};

class TypeConversionException : public MI::Exception
{
public:
    TypeConversionException() : MI::Exception(L"Unsupported type conversion") {}
};