// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <MI.h>

#ifdef _DEBUG
#define _DEBUG_TMP
#undef _DEBUG
#endif

#include <Python.h>
#include <structmember.h>

#ifdef _DEBUG_TMP
#define _DEBUG 1
#undef _DEBUG_TMP
#endif

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#define PYUNICODEASVARCHARARG1TYPE PyObject
#else
#define PYUNICODEASVARCHARARG1TYPE PyUnicodeObject

#define PyDateTime_DELTA_GET_DAYS(o)         (((PyDateTime_Delta*)o)->days)
#define PyDateTime_DELTA_GET_SECONDS(o)      (((PyDateTime_Delta*)o)->seconds)
#define PyDateTime_DELTA_GET_MICROSECONDS(o)            \
    (((PyDateTime_Delta*)o)->microseconds)

#endif

// TODO: reference additional headers your program requires here
