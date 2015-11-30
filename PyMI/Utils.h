#pragma once

#include <Python.h>
#include <mi.h>
#include <MIExceptions.h>
#include <string>
#include <functional>

PyObject* MI2Py(const MI_Value& value, MI_Type valueType, MI_Uint32 flags);
void Py2MI(PyObject* pyValue, MI_Value& value, MI_Type valueType);
void GetIndexOrName(PyObject *item, std::wstring& name, Py_ssize_t& i);
void SetPyException(const std::exception& ex);
void AllowThreads(std::function<void()> action);
void CallPythonCallback(PyObject* callable, const char* format, ...);
void MIIntervalFromPyDelta(PyObject* pyDelta, MI_Interval& interval);
PyObject* PyDeltaFromMIInterval(const MI_Interval& interval);

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