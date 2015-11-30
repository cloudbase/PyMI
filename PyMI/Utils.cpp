#include "stdafx.h"
#include "PyMI.h"
#include "Utils.h"
#include "instance.h"

#include <datetime.h>

void AllowThreads(std::function<void()> action)
{
    PyThreadState *_save;
    _save = PyEval_SaveThread();

    try
    {
        action();
        PyEval_RestoreThread(_save);
    }
    catch(std::exception&)
    {
        PyEval_RestoreThread(_save);
        throw;
    }
}

void CallPythonCallback(PyObject* callable, const char* format, ...)
{
    va_list vargs;

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    va_start(vargs, format);
    PyObject* arglist = Py_VaBuildValue(format, vargs);
    va_end(vargs);

    if (arglist)
    {
        PyObject* result = PyObject_CallObject(callable, arglist);
        Py_DECREF(arglist);
        if (result)
        {
            Py_DECREF(result);
        }
        else
        {
            PyErr_Print();
        }
    }
    else
    {
        PyErr_Print();
    }

    PyGILState_Release(gstate);
}

PyObject* PyDeltaFromMIInterval(const MI_Interval& interval)
{
    return PyDelta_FromDSU(interval.days, interval.hours * 3600 + interval.minutes * 60 + interval.seconds, interval.microseconds);
}

void MIIntervalFromPyDelta(PyObject* pyDelta, MI_Interval& interval)
{
    ZeroMemory(&interval, sizeof(MI_Interval));

    int days = PyDateTime_DELTA_GET_DAYS(pyDelta);
    if (days < 0)
    {
        throw MI::Exception(L"Negative datetime.timedelta intervals are not supported");
    }

    interval.days = (MI_Uint32)days;
    MI_Uint32 daySeconds = (MI_Uint32)PyDateTime_DELTA_GET_SECONDS(pyDelta);
    interval.hours = daySeconds / 3600;
    interval.minutes = (daySeconds - interval.hours * 3600) / 60;
    interval.seconds = daySeconds - interval.hours * 3600 - interval.minutes * 60;
    interval.microseconds = (MI_Uint32)PyDateTime_DELTA_GET_MICROSECONDS(pyDelta);
}

void GetIndexOrName(PyObject *item, std::wstring& name, Py_ssize_t& i)
{
    name.clear();
    i = -1;

#ifndef IS_PY3K
    if (PyString_Check(item))
    {
        char* s = PyString_AsString(item);
        DWORD len = lstrlenA(s) + 1;
        wchar_t* w = new wchar_t[len];

        if (::MultiByteToWideChar(CP_ACP, 0, s, len, w, len) != len)
        {
            delete [] w;
            throw MI::Exception(L"MultiByteToWideChar failed");
        }
        name.assign(w, len);
        delete[] w;
    }
    else
#endif
    if (PyUnicode_Check(item))
    {
        Py_ssize_t len = PyUnicode_GetSize(item) + 1;
        wchar_t* w = new wchar_t[len];

        if (PyUnicode_AsWideChar((PYUNICODEASVARCHARARG1TYPE*)item, w, len) < 0)
        {
            delete[] w;
            throw MI::Exception(L"PyUnicode_AsWideChar failed");
        }
        name.assign(w, len);
        delete[] w;
    }
    else if (PyIndex_Check(item))
    {
        i = PyNumber_AsSsize_t(item, PyExc_IndexError);
        if (i == -1 && PyErr_Occurred())
            throw MI::Exception(L"Index error");
    }
    else
        throw MI::Exception(L"Invalid name or index");
}

void Py2MIString(PyObject* pyValue, MI_Value& value)
{
    PyObject* pyStrValue = PyObject_CallMethod(pyValue, "__str__", NULL);
    if (!pyStrValue)
    {
        throw MI::Exception(L"PyObject_CallMethod failed for __str__");
    }

    try
    {
        Py2MI(pyStrValue, value, MI_STRING);
        Py_DECREF(pyStrValue);
    }
    catch (std::exception&)
    {
        Py_DECREF(pyStrValue);
        throw;
    }
}

unsigned GetItemSize(MI_Type valueType)
{
    switch (valueType)
    {
    case MI_BOOLEAN:
        return sizeof(MI_Boolean);
    case MI_SINT8:
        return sizeof(MI_Sint8);
    case MI_UINT8:
        return sizeof(MI_Uint8);
    case MI_SINT16:
        return sizeof(MI_Sint16);
    case MI_UINT16:
        return sizeof(MI_Uint16);
    case MI_SINT32:
        return sizeof(MI_Sint32);
    case MI_UINT32:
        return sizeof(MI_Uint32);
    case MI_SINT64:
        return sizeof(MI_Sint64);
    case MI_UINT64:
        return sizeof(MI_Uint64);
    case MI_REAL32:
        return sizeof(MI_Real32);
    case MI_REAL64:
        return sizeof(MI_Real64);
    case MI_CHAR16:
        return sizeof(MI_Char16);
    case MI_DATETIME:
        return sizeof(MI_Datetime);
    case MI_STRING:
        return sizeof(MI_Char*);
    case MI_INSTANCE:
    case MI_REFERENCE:
        return sizeof(MI_Instance*);
    default:
        throw TypeConversionException();
    }
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
        case MI_BOOLEAN:
            value.boolean = (MI_Boolean)(PyLong_AsLong(pyValue) != 0);
            break;
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
        case MI_CHAR16:
            value.char16 = (MI_Char16)PyLong_AsLong(pyValue);
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
        case MI_REAL32:
            value.real32 = (float)PyLong_AsDouble(pyValue);
            break;
        case MI_REAL64:
            value.real64 = PyLong_AsDouble(pyValue);
            break;
        case MI_STRING:
            Py2MIString(pyValue, value);
            break;
        default:
            throw TypeConversionException();
        }
    }
#ifndef IS_PY3K
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyInt_Type)))
    {
        switch (valueType)
        {
        case MI_BOOLEAN:
            value.boolean = (MI_Boolean)(PyInt_AsLong(pyValue) != 0);
            break;
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
        case MI_CHAR16:
            value.char16 = (MI_Char16)PyLong_AsLong(pyValue);
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
        case MI_REAL32:
            value.real32 = (float)PyInt_AsLong(pyValue);
            break;
        case MI_REAL64:
            value.real64 = PyInt_AsLong(pyValue);
            break;
        case MI_STRING:
            Py2MIString(pyValue, value);
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
            len = lstrlenA(s) + 1;
            // Note: caller owns the memory
            value.string = (MI_Char*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(MI_Char));
            if (!value.string)
            {
                throw OutOfMemoryException();
            }
            if (::MultiByteToWideChar(CP_ACP, 0, s, len, value.string, len) != len)
            {
                HeapFree(GetProcessHeap(), 0, value.string);
                value.string = NULL;
                throw MI::Exception(L"MultiByteToWideChar failed");
            }
            break;
        default:
            throw TypeConversionException();
        }
    }
#endif
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyUnicode_Type)))
    {
        Py_ssize_t len = 0;

        switch (valueType)
        {
        case MI_STRING:
            // TODO: use PyUnicode_GetLength on 3.x
            len = PyUnicode_GetSize(pyValue) + 1;
            // Note: caller owns the memory
            value.string = (MI_Char*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(MI_Char));
            if (!value.string)
            {
                throw OutOfMemoryException();
            }

            if (PyUnicode_AsWideChar((PYUNICODEASVARCHARARG1TYPE*)pyValue, value.string, len) < 0)
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

        MI_Type itemType = (MI_Type)(valueType ^ MI_ARRAY);
        unsigned itemSize = GetItemSize(itemType);

        // All array members of the MI_Value union have "pointer", "size" members.
        // It is safe to rely on one instead of referencing value.stringa, value.booleana, etc
        value.uint8a.size = (unsigned)size;
        value.uint8a.data = (MI_Uint8*)HeapAlloc(GetProcessHeap(), 0, itemSize * size);

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

            Py2MI(pyObj, tmpVal, itemType);
            memcpy(&value.uint8a.data[i * itemSize], &tmpVal, itemSize);
        }
    }
    else
    {
        switch (valueType)
        {
        case MI_STRING:
            Py2MIString(pyValue, value);
            break;
        default:
            throw TypeConversionException();
        }
    }
}

PyObject* MIArray2PyTuple(const MI_Value& value, MI_Type itemType)
{
    // All array members of the MI_Value union have "pointer", "size" members.
    // It is safe to rely on one instead of referencing value.stringa, value.booleana, etc
    MI_Uint32 size = value.uint8a.size;
    PyObject* pyObj = PyTuple_New(size);
    unsigned itemSize = GetItemSize(itemType);
    for (MI_Uint32 i = 0; i < size; i++)
    {
        MI_Value tmpVal;
        memcpy(&tmpVal, &value.uint8a.data[i * itemSize], itemSize);
        if (PyTuple_SetItem(pyObj, i, MI2Py(tmpVal, itemType, 0)))
        {
            Py_DECREF(pyObj);
            throw MI::Exception(L"PyTuple_SetItem failed");
        }
    }
    return pyObj;
}

PyObject* MI2Py(const MI_Value& value, MI_Type valueType, MI_Uint32 flags)
{
    if (flags & MI_FLAG_NULL)
        Py_RETURN_NONE;

    if (valueType & MI_ARRAY)
    {
        return MIArray2PyTuple(value, (MI_Type)(valueType ^ MI_ARRAY));
    }

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
        // TODO: understand where this import needs to be called
        PyDateTime_IMPORT;
        if (value.datetime.isTimestamp)
        {
            const MI_Timestamp& ts = value.datetime.u.timestamp;
            // TODO: Add timezone support!
            return PyDateTime_FromDateAndTime(ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.microseconds);
        }
        else
        {
            return PyDeltaFromMIInterval(value.datetime.u.interval);
        }
        break;
    case MI_STRING:
        return PyUnicode_FromWideChar(value.string, wcslen(value.string));
    case MI_INSTANCE:
        return (PyObject*)Instance_New(new MI::Instance(value.instance, false));
    case MI_REFERENCE:
        return (PyObject*)Instance_New(new MI::Instance(value.reference, false));
    default:
        throw TypeConversionException();
    }
}

void SetPyException(const std::exception& ex)
{
    const char* message = ex.what();
    if (reinterpret_cast<const MI::MITimeoutException*>(&ex))
    {
        PyErr_SetString(PyMITimeoutError, message);
    }
    else
    {
        PyErr_SetString(PyMIError, message);
    }
}
