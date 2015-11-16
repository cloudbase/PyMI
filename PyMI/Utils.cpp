#include "stdafx.h"
#include "PyMI.h"
#include "Utils.h"
#include "instance.h"

#include <datetime.h>


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

    if (pyValue == Py_None)
    {
        return;
    }
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
            throw TypeConversionException();
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
            throw TypeConversionException();
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
                throw OutOfMemoryException();
            }
            if (::MultiByteToWideChar(CP_ACP, 0, s, len + 1, value.string, len + 1) != len + 1)
            {
                throw MI::Exception(L"MultiByteToWideChar failed");
            }
            break;
        default:
            throw TypeConversionException();
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
                throw OutOfMemoryException();
            }

            if (PyUnicode_AsWideChar((PyUnicodeObject*)pyValue, value.string, len + 1) < 0)
            {
                throw MI::Exception(L"PyUnicode_AsWideChar failed");
            }
            break;
        default:
            throw TypeConversionException();
        }
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&InstanceType)))
    {
        switch (valueType)
        {
        case MI_REFERENCE:
            value.reference = ((Instance*)pyValue)->instance->GetMIObject();
            break;
        default:
            throw TypeConversionException();
        }
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyTuple_Type)) ||
             PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyList_Type)))
    {
        bool isTuple = PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyTuple_Type)) != 0;
        Py_ssize_t size = 0;
        if (isTuple)
        {
            size = PyTuple_Size(pyValue);
        }
        else
        {
            size = PyList_Size(pyValue);
        }

        if (size == 0)
            return;

        switch (valueType)
        {
        case MI_UINT32A:
            value.uint32a.size = size;
            value.uint32a.data = (MI_Uint32*)HeapAlloc(GetProcessHeap(), 0, sizeof(MI_Uint32) * size);
            break;
        case MI_STRINGA:
            value.stringa.size = size;
            value.stringa.data = (MI_Char**)HeapAlloc(GetProcessHeap(), 0, sizeof(MI_Char*) * size);
            break;
        case MI_REFERENCEA:
            value.referencea.size = size;
            value.referencea.data = (MI_Instance**)HeapAlloc(GetProcessHeap(), 0, sizeof(MI_Instance*) * size);
            break;
        default:
            throw TypeConversionException();
        }

        for (Py_ssize_t i = 0; i < size; i++)
        {
            MI_Value tmpVal;
            PyObject* pyObj = NULL;

            if (isTuple)
            {
                pyObj = PyTuple_GetItem(pyValue, i);
            }
            else
            {
                pyObj = PyList_GetItem(pyValue, i);
            }

            switch (valueType)
            {
            case MI_UINT32A:
                Py2MI(pyObj, tmpVal, MI_UINT32);
                value.uint32a.data[i] = tmpVal.uint32;
                break;
            case MI_STRINGA:
                Py2MI(pyObj, tmpVal, MI_STRING);
                value.stringa.data[i] = tmpVal.string;
                break;
            case MI_REFERENCEA:
                Py2MI(pyObj, tmpVal, MI_REFERENCE);
                value.referencea.data[i] = tmpVal.reference;
                break;
            default:
                throw TypeConversionException();
            }
        }
    }
    else
    {
        throw TypeConversionException();
    }
}

PyObject* MI2Py(const MI_Value& value, MI_Type valueType, MI_Uint32 flags)
{
    if (flags & MI_FLAG_NULL)
        Py_RETURN_NONE;

    size_t len = 0;
    PyObject* pyObj = NULL;

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
        return NULL;
    case MI_UINT16A:
        pyObj = PyTuple_New(value.uint16a.size);
        for (MI_Uint32 i = 0; i < value.uint16a.size; i++)
        {
            MI_Value tmpVal;
            tmpVal.uint16 = value.uint16a.data[i];
            if (PyTuple_SetItem(pyObj, i, MI2Py(tmpVal, MI_UINT16, 0)))
            {
                Py_DECREF(pyObj);
                throw MI::Exception(L"PyTuple_SetItem failed");
            }
        }
        return pyObj;
    case MI_SINT32A:
        return NULL;
    case MI_UINT32A:
        pyObj = PyTuple_New(value.uint32a.size);
        for (MI_Uint32 i = 0; i < value.uint32a.size; i++)
        {
            MI_Value tmpVal;
            tmpVal.uint32 = value.uint32a.data[i];
            if (PyTuple_SetItem(pyObj, i, MI2Py(tmpVal, MI_UINT32, 0)))
            {
                Py_DECREF(pyObj);
                throw MI::Exception(L"PyTuple_SetItem failed");
            }
        }
        return pyObj;
    case MI_SINT64A:
        return NULL;
    case MI_UINT64A:
        return NULL;
    case MI_REAL32A:
        return NULL;
    case MI_REAL64A:
        return NULL;
    case MI_CHAR16A:
        return NULL;
    case MI_DATETIMEA:
        pyObj = PyTuple_New(value.datetimea.size);
        for (MI_Uint32 i = 0; i < value.datetimea.size; i++)
        {
            MI_Value tmpVal;
            tmpVal.datetime = value.datetimea.data[i];
            if (PyTuple_SetItem(pyObj, i, MI2Py(tmpVal, MI_DATETIME, 0)))
            {
                Py_DECREF(pyObj);
                throw MI::Exception(L"PyTuple_SetItem failed");
            }
        }
        return pyObj;
    case MI_STRINGA:
        pyObj = PyTuple_New(value.stringa.size);
        for (MI_Uint32 i = 0; i < value.stringa.size; i++)
        {
            MI_Value tmpVal;
            tmpVal.string = value.stringa.data[i];
            if (PyTuple_SetItem(pyObj, i, MI2Py(tmpVal, MI_STRING, 0)))
            {
                Py_DECREF(pyObj);
                throw MI::Exception(L"PyTuple_SetItem failed");
            }
        }
        return pyObj;
    case MI_INSTANCE:
        pyObj = (PyObject*)Instance_New(new MI::Instance(value.instance, false));
        return pyObj;
    case MI_REFERENCE:
        pyObj = (PyObject*)Instance_New(new MI::Instance(value.reference, false));
        return pyObj;
    case MI_INSTANCEA:
        pyObj = PyTuple_New(value.instancea.size);
        for (unsigned i = 0; i < value.instancea.size; i++)
        {
            MI_Value tmpVal;
            tmpVal.instance = value.instancea.data[i];
            if (PyTuple_SetItem(pyObj, i, MI2Py(tmpVal, MI_INSTANCE, 0)))
            {
                Py_DECREF(pyObj);
                throw MI::Exception(L"PyTuple_SetItem failed");
            }
        }
        return pyObj;
    case MI_REFERENCEA:
        pyObj = PyTuple_New(value.stringa.size);
        for (unsigned i = 0; i < value.referencea.size; i++)
        {
            MI_Value tmpVal;
            tmpVal.reference = value.referencea.data[i];
            if (PyTuple_SetItem(pyObj, i, MI2Py(tmpVal, MI_REFERENCE, 0)))
            {
                Py_DECREF(pyObj);
                throw MI::Exception(L"PyTuple_SetItem failed");
            }
        }
        return pyObj;
    default:
        return NULL;
    }
}

void SetPyException(const std::exception& ex)
{
    const char* message = ex.what();
    PyErr_SetString(PyMIError, message);
}
