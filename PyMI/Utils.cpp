#include "stdafx.h"
#include "Utils.h"

PyObject* MI2Py(const MI_Value& value, MI_Type valueType)
{
    size_t len = 0;

    switch (valueType)
    {
    case MI_BOOLEAN:
    case MI_SINT8:
    case MI_UINT8:
    case MI_SINT16:
    case MI_UINT16:
    case MI_SINT32:
    case MI_UINT32:
    case MI_SINT64:
    case MI_UINT64:
    case MI_REAL32:
    case MI_REAL64:
    case MI_CHAR16:
    case MI_DATETIME:
        return NULL;
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
        break;
    default:
        return NULL;
    }
}
