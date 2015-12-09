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

std::wstring Py2WString(PyObject* pyValue)
{
    auto len = PyUnicode_GetSize(pyValue) + 1;
    wchar_t* w = new wchar_t[len];

    if (PyUnicode_AsWideChar((PYUNICODEASVARCHARARG1TYPE*)pyValue, w, len) < 0)
    {
        delete[] w;
        throw MI::Exception(L"PyUnicode_AsWideChar failed");
    }

    auto& value = std::wstring(w, len);
    delete[] w;
    return value;
}

std::shared_ptr<MI::MIValue> Py2StrMIValue(PyObject* pyValue)
{
    PyObject* pyStrValue = PyObject_CallMethod(pyValue, "__str__", NULL);
    if (!pyStrValue)
    {
        throw MI::Exception(L"PyObject_CallMethod failed for __str__");
    }

    try
    {
#ifdef IS_PY3K
        auto& value = Py2WString(pyStrValue);
#else
        auto& value = std::string(PyString_AsString(pyStrValue));
#endif
        Py_DECREF(pyStrValue);
        return MI::MIValue::FromString(value);
    }
    catch (std::exception&)
    {
        Py_DECREF(pyStrValue);
        throw;
    }
}

std::shared_ptr<MI::MIValue> Py2MI(PyObject* pyValue, MI_Type valueType)
{
    if (pyValue == Py_None)
    {
        return std::make_shared<MI::MIValue>(valueType);
    }
    if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyBool_Type)))
    {
        return MI::MIValue::FromBoolean(PyObject_IsTrue(pyValue) ? MI_TRUE : MI_FALSE);
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyLong_Type)))
    {
        switch (valueType)
        {
        case MI_BOOLEAN:
            return MI::MIValue::FromBoolean(PyLong_AsLong(pyValue) != 0);
        case MI_UINT8:
            return MI::MIValue::FromUint8((MI_Uint8)PyLong_AsUnsignedLong(pyValue));
        case MI_SINT8:
            return MI::MIValue::FromSint8((MI_Sint8)PyLong_AsLong(pyValue));
        case MI_UINT16:
            return MI::MIValue::FromUint16((MI_Uint16)PyLong_AsUnsignedLong(pyValue));
        case MI_SINT16:
            return MI::MIValue::FromSint16((MI_Sint16)PyLong_AsLong(pyValue));
        case MI_CHAR16:
            return MI::MIValue::FromChar16((MI_Char16)PyLong_AsLong(pyValue));
        case MI_UINT32:
            return MI::MIValue::FromUint32(PyLong_AsUnsignedLong(pyValue));
        case MI_SINT32:
            return MI::MIValue::FromSint32(PyLong_AsLong(pyValue));
        case MI_UINT64:
            return MI::MIValue::FromUint64(PyLong_AsUnsignedLongLong(pyValue));
        case MI_SINT64:
            return MI::MIValue::FromSint64(PyLong_AsLongLong(pyValue));
        case MI_REAL32:
            return MI::MIValue::FromReal32((MI_Real32)PyLong_AsDouble(pyValue));
        case MI_REAL64:
            return MI::MIValue::FromReal64(PyLong_AsDouble(pyValue));
        case MI_STRING:
            return Py2StrMIValue(pyValue);
        default:
            throw MI::TypeConversionException();
        }
    }
#ifndef IS_PY3K
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyInt_Type)))
    {
        switch (valueType)
        {
        case MI_BOOLEAN:
            return MI::MIValue::FromBoolean(PyInt_AsLong(pyValue) != 0);
        case MI_UINT8:
            return MI::MIValue::FromUint8((MI_Uint8)PyInt_AsLong(pyValue));
        case MI_SINT8:
            return MI::MIValue::FromSint8((MI_Sint8)PyInt_AsLong(pyValue));
        case MI_UINT16:
            return MI::MIValue::FromUint16((MI_Uint16)PyInt_AsLong(pyValue));
        case MI_SINT16:
            return MI::MIValue::FromSint16((MI_Sint16)PyInt_AsLong(pyValue));
        case MI_CHAR16:
            return MI::MIValue::FromChar16((MI_Char16)PyLong_AsLong(pyValue));
        case MI_UINT32:
            return MI::MIValue::FromUint32((MI_Uint32)PyInt_AsLong(pyValue));
        case MI_SINT32:
            return MI::MIValue::FromSint32((MI_Sint32)PyInt_AsLong(pyValue));
        case MI_UINT64:
            return MI::MIValue::FromUint64((MI_Uint64)PyInt_AsLong(pyValue));
        case MI_SINT64:
            return MI::MIValue::FromSint64((MI_Sint64)PyInt_AsLong(pyValue));
        case MI_REAL32:
            return MI::MIValue::FromReal32((MI_Real32)PyInt_AsLong(pyValue));
        case MI_REAL64:
            return MI::MIValue::FromReal64((MI_Real64)PyInt_AsLong(pyValue));
        case MI_STRING:
            return Py2StrMIValue(pyValue);
        default:
            throw MI::TypeConversionException();
        }
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyString_Type)))
    {
        switch (valueType)
        {
        case MI_STRING:
            return MI::MIValue::FromString(std::string(PyString_AsString(pyValue)));
        default:
            throw MI::TypeConversionException();
        }
    }
#endif
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&PyUnicode_Type)))
    {
        switch (valueType)
        {
        case MI_STRING:
            return MI::MIValue::FromString(Py2WString(pyValue));
        default:
            throw MI::TypeConversionException();
        }
    }
    else if (PyObject_IsInstance(pyValue, reinterpret_cast<PyObject*>(&InstanceType)))
    {
        switch (valueType)
        {
        case MI_INSTANCE:
            return MI::MIValue::FromInstance(*((Instance*)pyValue)->instance);
        case MI_REFERENCE:
            return MI::MIValue::FromReference(*((Instance*)pyValue)->instance);
        default:
            throw MI::TypeConversionException();
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

        MI_Type itemType = (MI_Type)(valueType ^ MI_ARRAY);
        auto value = MI::MIValue::CreateArray((unsigned)size, valueType);

        for (Py_ssize_t i = 0; i < size; i++)
        {
            PyObject* pyObj = NULL;
            if (isTuple)
            {
                pyObj = PyTuple_GetItem(pyValue, i);
            }
            else
            {
                pyObj = PyList_GetItem(pyValue, i);
            }

            auto& tmpValue = Py2MI(pyObj, itemType);
            value->SetArrayItem(*tmpValue, (unsigned)i);
        }
        return value;
    }
    else
    {
        switch (valueType)
        {
        case MI_STRING:
            return Py2StrMIValue(pyValue);
        default:
            throw MI::TypeConversionException();
        }
    }
}

PyObject* MIArray2PyTuple(const MI_Value& value, MI_Type itemType)
{
    // All array members of the MI_Value union have "pointer", "size" members.
    // It is safe to rely on one instead of referencing value.stringa, value.booleana, etc
    MI_Uint32 size = value.uint8a.size;
    PyObject* pyObj = PyTuple_New(size);
    unsigned itemSize = MI::MIValue::GetItemSize(itemType);
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
        return (PyObject*)Instance_New(std::make_shared<MI::Instance>(value.instance, false));
    case MI_REFERENCE:
        return (PyObject*)Instance_New(std::make_shared<MI::Instance>(value.reference, false));
    default:
        throw MI::TypeConversionException();
    }
}

void SetPyException(const std::exception& ex)
{
    const char* message = ex.what();
    if (dynamic_cast<const MI::MITimeoutException*>(&ex))
    {
        PyErr_SetString(PyMITimeoutError, message);
    }
    else if (dynamic_cast<const MI::TypeConversionException*>(&ex))
    {
        PyErr_SetString(PyExc_TypeError, message);
    }
    else
    {
        PyErr_SetString(PyMIError, message);
    }
}
