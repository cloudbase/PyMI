// PyMI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PyMI.h"
#include "Application.h"
#include "Session.h"
#include "Class.h"
#include "Operation.h"
#include "Instance.h"

#include <datetime.h>

PyObject *PyMIError;

static PyMethodDef mi_methods[] = {
    { NULL, NULL, 0, NULL }  /* Sentinel */
};

PyMODINIT_FUNC initmi(void)
{
    PyDateTime_IMPORT;

    PyObject* m = NULL;

    ApplicationType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ApplicationType) < 0)
        return;

    SessionType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&SessionType) < 0)
        return;

    ClassType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ClassType) < 0)
        return;

    InstanceType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&InstanceType) < 0)
        return;

    OperationType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&OperationType) < 0)
        return;

    m = Py_InitModule3("mi", mi_methods, "MI module.");
    if (m == NULL)
        return;

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

    /*
    m = Py_InitModule("mi", PyMIMethods);
    if (m == NULL)
        return;
    */

    PyMIError = PyErr_NewException("PyMI.error", NULL, NULL);
    Py_INCREF(PyMIError);
    PyModule_AddObject(m, "error", PyMIError);
}
