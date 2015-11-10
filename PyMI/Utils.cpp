#include "stdafx.h"
#include "Utils.h"

#include <datetime.h>

PyObject* MI2Py(const MI_Value& value, MI_Type valueType, MI_Uint32 flags)
{
    if (flags & MI_FLAG_NULL)
        return Py_None;

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
        break;
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
