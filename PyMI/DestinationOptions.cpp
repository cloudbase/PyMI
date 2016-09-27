#include "stdafx.h"
#include "DestinationOptions.h"
#include "PyMI.h"
#include "Utils.h"

#include <datetime.h>


static PyObject* DestinationOptions_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    DestinationOptions* self = NULL;
    sizeof(DestinationOptions);
    self = (DestinationOptions*)type->tp_alloc(type, 0);
    self->destinationOptions = NULL;
    ::InitializeCriticalSection(&self->cs);
    return (PyObject *)self;
}

static int DestinationOptions_init(DestinationOptions* self, PyObject* args, PyObject* kwds)
{
    PyErr_SetString(PyMIError, "An DestinationOptions object cannot be allocated directly.");
    return -1;
}

static void DestinationOptions_dealloc(DestinationOptions* self)
{
    AllowThreads(&self->cs, [&]() {
        self->destinationOptions = NULL;
    });
    ::DeleteCriticalSection(&self->cs);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

DestinationOptions* DestinationOptions_New(std::shared_ptr<MI::DestinationOptions> destinationOptions)
{
    DestinationOptions* obj = (DestinationOptions*)DestinationOptions_new(&DestinationOptionsType, NULL, NULL);
    obj->destinationOptions = destinationOptions;
    return obj;
}

static PyObject* DestinationOptions_Clone(DestinationOptions *self, PyObject*)
{
    try
    {
        std::shared_ptr<MI::DestinationOptions> destinationOptions;
        AllowThreads(&self->cs, [&]() {
            destinationOptions = self->destinationOptions->Clone();
        });
        return (PyObject*)DestinationOptions_New(destinationOptions);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* DestinationOptions_GetUILocale(DestinationOptions* self)
{
    try
    {
        std::wstring locale;
        AllowThreads(&self->cs, [&]() {
            locale = self->destinationOptions->GetUILocale();
        });
        const std::string sTmp(locale.begin(), locale.end());
        return PyUnicode_FromString(sTmp.c_str());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* DestinationOptions_SetUILocale(DestinationOptions* self, PyObject *args, PyObject *kwds)
{
    wchar_t* locale = NULL;
    static char *kwlist[] = { "locale_name", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "u", kwlist, &locale))
        return NULL;

    try
    {
        AllowThreads(&self->cs, [&]() {
            self->destinationOptions->SetUILocale(locale);
        });
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* DestinationOptions_GetTimeout(DestinationOptions* self, PyObject* timeout)
{
    try
    {
        MI_Interval interval;
        AllowThreads(&self->cs, [&]() {
            interval = self->destinationOptions->GetTimeout();
        });
        return PyDeltaFromMIInterval(interval);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* DestinationOptions_SetTimeout(DestinationOptions* self, PyObject* timeout)
{
    PyDateTime_IMPORT;

    if (!PyDelta_Check(timeout))
    {
        PyErr_SetString(PyExc_TypeError, "parameter timeout must be of type datetime.timedelta");
        return NULL;
    }

    try
    {
        MI_Interval miTimeout;
        MIIntervalFromPyDelta(timeout, miTimeout);
        AllowThreads(&self->cs, [&]() {
            self->destinationOptions->SetTimeout(miTimeout);
        });
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* DestinationOptions_SetTransport(DestinationOptions* self, PyObject *args, PyObject *kwds)
{
    wchar_t* transport = NULL;
    static char *kwlist[] = { "transport", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "u", kwlist, &transport))
        return NULL;

    try
    {
        AllowThreads(&self->cs, [&]() {
            self->destinationOptions->SetTransport(transport);
        });
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* DestinationOptions_GetTransport(DestinationOptions* self)
{
    try
    {
        std::wstring transport;
        AllowThreads(&self->cs, [&]() {
            transport = self->destinationOptions->GetTransport();
        });
        const std::string sTmp(transport.begin(), transport.end());
        return PyUnicode_FromString(sTmp.c_str());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* DestinationOptions_AddCredentials(DestinationOptions* self, PyObject *args, PyObject *kwds)
{
    wchar_t* authType = NULL;
    wchar_t* domain = NULL;
    wchar_t* username = NULL;
    wchar_t* password = NULL;
    wchar_t* certThumbprint = NULL;

    static char *kwlist[] = { "auth_type", "domain", "username", "password", "cert_thumbprint", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "u|uuuu", kwlist,
                                     &authType, &domain, &username, &password, &certThumbprint))
        return NULL;

    try
    {
        if (certThumbprint && wcslen(certThumbprint))
            AllowThreads(&self->cs, [&]() {
                self->destinationOptions->AddCredentials(authType, certThumbprint);
            });
        else
            AllowThreads(&self->cs, [&]() {
                self->destinationOptions->AddCredentials(authType, domain, username, password);
            });
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}


static PyMemberDef DestinationOptions_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef DestinationOptions_methods[] = {
    { "clone", (PyCFunction)DestinationOptions_Clone, METH_NOARGS, "Clones the DestinationOptions." },
    { "get_ui_locale", (PyCFunction)DestinationOptions_GetUILocale, METH_NOARGS, "Returns the UI locale." },
    { "set_ui_locale", (PyCFunction)DestinationOptions_SetUILocale, METH_VARARGS | METH_KEYWORDS, "Sets the UI locale." },
    { "get_transport", (PyCFunction)DestinationOptions_GetTransport, METH_VARARGS | METH_KEYWORDS, "Gets the transport protocol." },
    { "set_transport", (PyCFunction)DestinationOptions_SetTransport, METH_VARARGS | METH_KEYWORDS, "Sets the transport protocol." },
    { "get_timeout", (PyCFunction)DestinationOptions_GetTimeout, METH_NOARGS, "Returns the default operation timeout." },
    { "set_timeout", (PyCFunction)DestinationOptions_SetTimeout, METH_O, "Sets the default operation timeout." },
    { "add_credentials", (PyCFunction)DestinationOptions_AddCredentials, METH_VARARGS | METH_KEYWORDS, "Adds credentials." },
    { NULL }  /* Sentinel */
};

PyTypeObject DestinationOptionsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mi.destinationoptions",     /*tp_name*/
    sizeof(DestinationOptions),  /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)DestinationOptions_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "DestinationOptions objects",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    DestinationOptions_methods,             /* tp_methods */
    DestinationOptions_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)DestinationOptions_init,    /* tp_init */
    0,                         /* tp_alloc */
    DestinationOptions_new,                 /* tp_new */
};
