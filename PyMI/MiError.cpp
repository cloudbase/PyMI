#include "stdafx.h"
#include "MiError.h"
#include "PyMI.h"

#include <MIExceptions.h>

PyObject* MiError_Init(void)
{
    PyObject* m = NULL;

#ifdef IS_PY3K
    m = PyModule_Create(&mi_error_module);
    if (m == NULL)
        return NULL;
#else
    m = Py_InitModule3("mi_error", mi_error_methods, "MI errors module.");
    if (m == NULL)
        return NULL;
#endif

    for (int i = 0; i < sizeof(MI::MI_RESULT_STRINGS) / sizeof(MI::MI_RESULT_STRINGS[0]); i++)
    {
        PyObject* err_str = PyUnicode_FromWideChar(MI::MI_RESULT_STRINGS[i],
                                                   wcslen(MI::MI_RESULT_STRINGS[i]));
        PyObject* err_code = PyLong_FromLong(i);
        PyObject_SetAttr(m, err_str, err_code);
    }

    return m;
}
