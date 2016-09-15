// PyMI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PyMI.h"
#include "Application.h"
#include "Session.h"
#include "Class.h"
#include "Operation.h"
#include "Instance.h"
#include "Serializer.h"
#include "OperationOptions.h"
#include "DestinationOptions.h"
#include "MiError.h"

#include <datetime.h>

PyObject *PyMIError;
PyObject *PyMITimeoutError;

static PyMethodDef mi_methods[] = {
    { NULL, NULL, 0, NULL }  /* Sentinel */
};

#ifdef IS_PY3K
static PyModuleDef mimodule = {
    PyModuleDef_HEAD_INIT,
    "mi",
    "Management Infrastructure API module.",
    -1,
    mi_methods
};
#endif

PyObject* _initmi(void)
{
    if (!PyEval_ThreadsInitialized())
    {
        PyEval_InitThreads();
    }

    PyDateTime_IMPORT;

    PyObject* m = NULL;

    if (PyType_Ready(&ApplicationType) < 0)
        return NULL;

    if (PyType_Ready(&SessionType) < 0)
        return NULL;

    if (PyType_Ready(&ClassType) < 0)
        return NULL;

    if (PyType_Ready(&InstanceType) < 0)
        return NULL;

    if (PyType_Ready(&OperationType) < 0)
        return NULL;

    if (PyType_Ready(&SerializerType) < 0)
        return NULL;

    if (PyType_Ready(&OperationOptionsType) < 0)
        return NULL;

    if (PyType_Ready(&DestinationOptionsType) < 0)
        return NULL;

#ifdef IS_PY3K
    m = PyModule_Create(&mimodule);
    if (m == NULL)
        return NULL;
#else
    m = Py_InitModule3("mi", mi_methods, "MI module.");
    if (m == NULL)
        return NULL;
#endif

    Py_INCREF(&ApplicationType);
    PyModule_AddObject(m, "Application", (PyObject*)&ApplicationType);

    Py_INCREF(&SessionType);
    PyModule_AddObject(m, "Session", (PyObject*)&SessionType);

    Py_INCREF(&ClassType);
    PyModule_AddObject(m, "Class", (PyObject*)&ClassType);

    Py_INCREF(&InstanceType);
    PyModule_AddObject(m, "Instance", (PyObject*)&InstanceType);

    Py_INCREF(&OperationType);
    PyModule_AddObject(m, "Operation", (PyObject*)&OperationType);

    Py_INCREF(&SerializerType);
    PyModule_AddObject(m, "Serializer", (PyObject*)&SerializerType);

    Py_INCREF(&OperationOptionsType);
    PyModule_AddObject(m, "OperationOptions", (PyObject*)&OperationOptionsType);

    Py_INCREF(&DestinationOptionsType);
    PyModule_AddObject(m, "DestinationOptions", (PyObject*)&DestinationOptionsType);

    PyMIError = PyErr_NewException("PyMI.error", NULL, NULL);
    Py_INCREF(PyMIError);
    PyModule_AddObject(m, "error", PyMIError);

    PyMITimeoutError = PyErr_NewException("PyMI.timeouterror", PyMIError, NULL);
    Py_INCREF(PyMITimeoutError);
    PyModule_AddObject(m, "timeouterror", PyMITimeoutError);

    PyObject_SetAttrString(m, "PROTOCOL_WINRM", PyUnicode_FromString("WINRM"));
    PyObject_SetAttrString(m, "PROTOCOL_WMIDCOM", PyUnicode_FromString("WMIDCOM"));

    PyObject_SetAttrString(m, "MI_BOOLEAN", PyLong_FromLong(MI_BOOLEAN));
    PyObject_SetAttrString(m, "MI_UINT8", PyLong_FromLong(MI_UINT8));
    PyObject_SetAttrString(m, "MI_SINT8", PyLong_FromLong(MI_SINT8));
    PyObject_SetAttrString(m, "MI_UINT16", PyLong_FromLong(MI_UINT16));
    PyObject_SetAttrString(m, "MI_SINT16", PyLong_FromLong(MI_SINT16));
    PyObject_SetAttrString(m, "MI_UINT32", PyLong_FromLong(MI_UINT32));
    PyObject_SetAttrString(m, "MI_SINT32", PyLong_FromLong(MI_SINT32));
    PyObject_SetAttrString(m, "MI_UINT64", PyLong_FromLong(MI_UINT64));
    PyObject_SetAttrString(m, "MI_SINT64", PyLong_FromLong(MI_SINT64));
    PyObject_SetAttrString(m, "MI_REAL32", PyLong_FromLong(MI_REAL32));
    PyObject_SetAttrString(m, "MI_REAL64", PyLong_FromLong(MI_REAL64));
    PyObject_SetAttrString(m, "MI_CHAR16", PyLong_FromLong(MI_CHAR16));
    PyObject_SetAttrString(m, "MI_DATETIME", PyLong_FromLong(MI_DATETIME));
    PyObject_SetAttrString(m, "MI_STRING", PyLong_FromLong(MI_STRING));
    PyObject_SetAttrString(m, "MI_REFERENCE", PyLong_FromLong(MI_REFERENCE));
    PyObject_SetAttrString(m, "MI_INSTANCE", PyLong_FromLong(MI_INSTANCE));
    PyObject_SetAttrString(m, "MI_UINT8A", PyLong_FromLong(MI_UINT8A));
    PyObject_SetAttrString(m, "MI_SINT8A", PyLong_FromLong(MI_SINT8A));
    PyObject_SetAttrString(m, "MI_UINT16A", PyLong_FromLong(MI_UINT16A));
    PyObject_SetAttrString(m, "MI_SINT16A", PyLong_FromLong(MI_SINT16A));
    PyObject_SetAttrString(m, "MI_UINT32A", PyLong_FromLong(MI_UINT32A));
    PyObject_SetAttrString(m, "MI_SINT32A", PyLong_FromLong(MI_SINT32A));
    PyObject_SetAttrString(m, "MI_UINT64A", PyLong_FromLong(MI_UINT64A));
    PyObject_SetAttrString(m, "MI_SINT64A", PyLong_FromLong(MI_SINT64A));
    PyObject_SetAttrString(m, "MI_REAL32A", PyLong_FromLong(MI_REAL32A));
    PyObject_SetAttrString(m, "MI_REAL64A", PyLong_FromLong(MI_REAL64A));
    PyObject_SetAttrString(m, "MI_CHAR16A", PyLong_FromLong(MI_CHAR16A));
    PyObject_SetAttrString(m, "MI_DATETIMEA", PyLong_FromLong(MI_DATETIMEA));
    PyObject_SetAttrString(m, "MI_STRINGA", PyLong_FromLong(MI_STRINGA));
    PyObject_SetAttrString(m, "MI_REFERENCEA", PyLong_FromLong(MI_REFERENCEA));
    PyObject_SetAttrString(m, "MI_INSTANCEA", PyLong_FromLong(MI_INSTANCEA));
    PyObject_SetAttrString(m, "MI_ARRAY", PyLong_FromLong(MI_ARRAY));

    PyObject_SetAttrString(m, "MI_AUTH_TYPE_DEFAULT",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_DEFAULT,
                                                  wcslen(MI_AUTH_TYPE_DEFAULT)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_NONE",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_NONE,
                                                  wcslen(MI_AUTH_TYPE_NONE)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_DIGEST",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_DIGEST,
                                                  wcslen(MI_AUTH_TYPE_DIGEST)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_NEGO_WITH_CREDS",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_NEGO_WITH_CREDS,
                                                  wcslen(MI_AUTH_TYPE_NEGO_WITH_CREDS)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_NEGO_NO_CREDS",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_NEGO_NO_CREDS,
                                                  wcslen(MI_AUTH_TYPE_NEGO_NO_CREDS)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_BASIC",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_BASIC,
                                                  wcslen(MI_AUTH_TYPE_BASIC)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_KERBEROS",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_KERBEROS,
                                                  wcslen(MI_AUTH_TYPE_KERBEROS)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_CLIENT_CERTS",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_CLIENT_CERTS,
                                                  wcslen(MI_AUTH_TYPE_CLIENT_CERTS)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_NTLM",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_NTLM,
                                                  wcslen(MI_AUTH_TYPE_NTLM)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_CREDSSP",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_CREDSSP,
                                                  wcslen(MI_AUTH_TYPE_CREDSSP)));
    PyObject_SetAttrString(m, "MI_AUTH_TYPE_ISSUER_CERT",
                           PyUnicode_FromWideChar(MI_AUTH_TYPE_ISSUER_CERT,
                                                  wcslen(MI_AUTH_TYPE_ISSUER_CERT)));

    PyObject_SetAttrString(m, "MI_TRANSPORT_HTTP",
                           PyUnicode_FromWideChar(MI_DESTINATIONOPTIONS_TRANSPORT_HTTP,
                                                  wcslen(MI_DESTINATIONOPTIONS_TRANSPORT_HTTP)));
    // The misspelling of 'transport' is intentional, as this is how it's defined by MI.h.
    PyObject_SetAttrString(m, "MI_TRANSPORT_HTTPS",
                           PyUnicode_FromWideChar(MI_DESTINATIONOPTIONS_TRANPSORT_HTTPS,
                                                  wcslen(MI_DESTINATIONOPTIONS_TRANPSORT_HTTPS)));

    PyObject* mi_error = MiError_Init();
    if (mi_error == NULL)
        return NULL;
    else
        PyObject_SetAttrString(m, "mi_error", mi_error);

    return m;
}

#ifdef IS_PY3K
PyMODINIT_FUNC PyInit_mi(void)
#else
PyMODINIT_FUNC initmi(void)
#endif
{
    PyObject* m = _initmi();
#ifdef IS_PY3K
    return m;
#endif
}
