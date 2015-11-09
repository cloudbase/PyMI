// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <mi.h>

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


// TODO: reference additional headers your program requires here
