#include "stdafx.h"
#include "Utils.h"

#include <datetime.h>
#include <exception>


bool GetIndexOrName(PyObject *item, wchar_t* w, Py_ssize_t& i)
{
    w[0] = NULL;
    i = -1;

    if (PyString_Check(item))
    {
        char* s = PyString_AsString(item);
        ::MultiByteToWideChar(CP_ACP, 0, s, -1, w, 1024);
    }
    else if (PyUnicode_Check(item))
    {
        if (PyUnicode_AsWideChar((PyUnicodeObject*)item, w, 1024) < 0)
            return false;
    }
    else if (PyIndex_Check(item))
    {
        i = PyNumber_AsSsize_t(item, PyExc_IndexError);
        if (i == -1 && PyErr_Occurred())
            return false;
    }
    else
        return false;

    return true;
}

void Py2MI(PyObject* pyValue, MI_Value& value, MI_Type valueType)
{
    ZeroMemory(&value, sizeof(value));

    if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyBool_Type)))
    {
        value.boolean = PyObject_IsTrue(pyValue) ? MI_TRUE : MI_FALSE;
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyLong_Type)))
    {
        switch (valueType)
        {
        case MI_UINT8:
            value.uint8 = (MI_Uint8)PyLong_AsUnsignedLong(pyValue);
            break;
        case MI_SINT8:
            value.sint8 = (MI_Sint8)PyLong_AsLong(pyValue);
            break;
        case MI_UINT16:
            value.uint16 = (MI_Uint16)PyLong_AsUnsignedLong(pyValue);
            break;
        case MI_SINT16:
            value.sint16 = (MI_Sint16)PyLong_AsLong(pyValue);
            break;
        case MI_UINT32:
            value.uint32 = PyLong_AsUnsignedLong(pyValue);
            break;
        case MI_SINT32:
            value.sint32 = PyLong_AsLong(pyValue);
            break;
        case MI_UINT64:
            value.uint64 = PyLong_AsUnsignedLongLong(pyValue);
            break;
        case MI_SINT64:
            value.sint64 = PyLong_AsLongLong(pyValue);
            break;
        default:
            throw std::exception("Unsupported type conversion");
        }
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyInt_Type)))
    {
        switch (valueType)
        {
        case MI_UINT8:
            value.uint8 = (MI_Uint8)PyInt_AsLong(pyValue);
            break;
        case MI_SINT8:
            value.sint8 = (MI_Sint8)PyInt_AsLong(pyValue);
            break;
        case MI_UINT16:
            value.uint16 = (MI_Uint16)PyInt_AsLong(pyValue);
            break;
        case MI_SINT16:
            value.sint16 = (MI_Sint16)PyInt_AsLong(pyValue);
            break;
        case MI_UINT32:
            value.uint32 = PyInt_AsLong(pyValue);
            break;
        case MI_SINT32:
            value.sint32 = PyInt_AsLong(pyValue);
            break;
        case MI_UINT64:
            value.uint64 = PyInt_AsLong(pyValue);
            break;
        case MI_SINT64:
            value.sint64 = PyInt_AsLong(pyValue);
            break;
        default:
            throw std::exception("Unsupported type conversion");
        }
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyString_Type)))
    {
        unsigned len = 0;
        char* s = NULL;

        switch (valueType)
        {
        case MI_STRING:
            s = PyString_AsString(pyValue);
            len = lstrlenA(s);
            // Note: caller owns the memory
            value.string = (MI_Char*)HeapAlloc(GetProcessHeap(), 0, (len + 1) * sizeof(MI_Char));
            if (!value.string)
            {
                throw std::exception("Out of memory");
            }
            if (::MultiByteToWideChar(CP_ACP, 0, s, len, value.string, len) != len)
            {
                throw std::exception("MultiByteToWideChar");
            }
        default:
            throw std::exception("Unsupported type conversion");
        }
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyUnicode_Type)))
    {
        Py_ssize_t len = 0;

        switch (valueType)
        {
        case MI_STRING:
            // TODO: use PyUnicode_GetLength on 3.x
            len = PyUnicode_GetSize(pyValue);
            // Note: caller owns the memory
            value.string = (MI_Char*)HeapAlloc(GetProcessHeap(), 0, (len + 1) * sizeof(MI_Char));
            if (!value.string)
            {
                throw std::exception("Out of memory");
            }

            if (PyUnicode_AsWideChar((PyUnicodeObject*)pyValue, value.string, len) < 0)
            {
                throw std::exception("PyUnicode_AsWideChar");
            }
            break;
        default:
            throw std::exception("Unsupported type conversion");
        }
    }
    else
    {
        throw std::exception("Unsupported type conversion");
    }
}

PyObject* MI2Py(const MI_Value& value, MI_Type valueType, MI_Uint32 flags)
{
    if (flags & MI_FLAG_NULL)
        Py_RETURN_NONE;

    size_t len = 0;

    switch (valueType)
    {
    case MI_BOOLEAN:
        return PyBool_FromLong(value.boolean);
    case MI_SINT8:
        return PyLong_FromLong(value.sint8);
    case MI_UINT8:
        return PyLong_FromUnsignedLong(value.uint8);
    case MI_SINT16:
        return PyLong_FromLong(value.sint16);
    case MI_UINT16:
        return PyLong_FromUnsignedLong(value.uint16);
    case MI_SINT32:
        return PyLong_FromLong(value.sint32);
    case MI_UINT32:
        return PyLong_FromUnsignedLong(value.uint32);
    case MI_SINT64:
        return PyLong_FromLongLong(value.sint64);
    case MI_UINT64:
        return PyLong_FromUnsignedLongLong(value.uint64);
    case MI_REAL32:
        return PyFloat_FromDouble(value.real32);
    case MI_REAL64:
        return PyFloat_FromDouble(value.real64);
    case MI_CHAR16:
        return PyLong_FromLong(value.char16);
    case MI_DATETIME:
        // TODO: understand where this needs to be set
        PyDateTime_IMPORT;
        if (value.datetime.isTimestamp)
        {
            const MI_Timestamp& ts = value.datetime.u.timestamp;
            // TODO: Add timezone support!
            return PyDateTime_FromDateAndTime(ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.microseconds);
        }
        else
        {
            const MI_Interval& ti = value.datetime.u.interval;
            return PyDelta_FromDSU(ti.days, ti.hours * 3600 + ti.minutes * 60 + ti.seconds, ti.microseconds);
        }
        break;
    case MI_STRING:
        len = wcslen(value.string);
        return PyUnicode_FromWideChar(value.string, len);
    case MI_BOOLEANA:
    case MI_SINT8A:
    case MI_UINT8A:
    case MI_SINT16A:
    case MI_UINT16A:
    case MI_SINT32A:
    case MI_UINT32A:
    case MI_SINT64A:
    case MI_UINT64A:
    case MI_REAL32A:
    case MI_REAL64A:
    case MI_CHAR16A:
    case MI_DATETIMEA:
    case MI_STRINGA:
    case MI_INSTANCE:
    case MI_REFERENCE:
    case MI_INSTANCEA:
    case MI_REFERENCEA:
        return NULL;
    default:
        return NULL;
    }
}
