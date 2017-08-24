#pragma once

#include <Python.h>
#include <MI.h>
#include <MIExceptions.h>
#include <string>
#include <functional>
#include <memory>
#include "mi++.h"

PyObject* MI2Py(const MI_Value& value, MI_Type valueType, MI_Uint32 flags);
std::shared_ptr<MI::MIValue> Py2MI(PyObject* pyValue, MI_Type valueType);
void GetIndexOrName(PyObject *item, std::wstring& name, Py_ssize_t& i);
void SetPyException(const std::exception& ex);
void AllowThreads(PCRITICAL_SECTION cs, std::function<void()> action);
void CallPythonCallback(PyObject* callable, const char* format, ...);
void MIIntervalFromPyDelta(PyObject* pyDelta, MI_Interval& interval);
PyObject* PyDeltaFromMIInterval(const MI_Interval& interval);
bool CheckPyNone(PyObject* obj);
void ValidatePyObjectType(PyObject* obj, const std::wstring& objName,
                          PyTypeObject* expectedType, const std::wstring& expectedTypeName,
                          bool allowNone = true);
