#include "stdafx.h"
#include "Application.h"
#include "Session.h"
#include "Class.h"
#include "Instance.h"
#include "Serializer.h"
#include "OperationOptions.h"
#include "DestinationOptions.h"
#include "Utils.h"


static PyObject* Application_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Application* self = NULL;
    self = (Application*)type->tp_alloc(type, 0);
    ::InitializeCriticalSection(&self->cs);
    return (PyObject *)self;
}

static int Application_init(Application *self, PyObject *args, PyObject *kwds)
{
    wchar_t* appId = L"";
    static char *kwlist[] = { "app_id", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|u", kwlist, &appId))
        return -1;

    try
    {
        AllowThreads(&self->cs, [&]() {
            self->app = std::make_shared<MI::Application>(appId);
        });
        return 0;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return -1;
    }
}

static PyObject* Application_NewSession(Application *self, PyObject *args, PyObject *kwds)
{
    wchar_t* protocol = L"";
    wchar_t* computerName = L".";
    PyObject* destinationOptions = NULL;

    static char *kwlist[] = { "protocol", "computer_name", "destination_options", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|uuO", kwlist, &protocol, &computerName, &destinationOptions))
        return NULL;

    try
    {
        if (!CheckPyNone(destinationOptions) && !PyObject_IsInstance(destinationOptions, reinterpret_cast<PyObject*>(&DestinationOptionsType)))
        {
            throw MI::TypeConversionException(L"\"destination_options\" must have type DestinationOptions");
        }

        std::shared_ptr<MI::Session> session;
        AllowThreads(&self->cs, [&]() {
            session = self->app->NewSession(protocol, computerName,
                !CheckPyNone(destinationOptions) ? ((DestinationOptions*)destinationOptions)->destinationOptions : NULL);
        });
        return (PyObject*)Session_New(session);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static void Application_dealloc(Application* self)
{
    if (self->app)
    {
        AllowThreads(&self->cs, [&]() {
            self->app = NULL;
        });
    }
    ::DeleteCriticalSection(&self->cs);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* Application_NewMethodInboundParameters(Application *self, PyObject *args, PyObject *kwds)
{
    PyObject* pyClass = NULL;
    wchar_t* methodName = NULL;

    static char *kwlist[] = { "mi_class", "method_name", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ou", kwlist, &pyClass, &methodName))
        return NULL;

    try
    {
        if (!PyObject_IsInstance(pyClass, reinterpret_cast<PyObject*>(&ClassType)))
            throw MI::TypeConversionException(L"\"mi_class\" must have type Class");

        std::shared_ptr<MI::Instance> instance;
        AllowThreads(&self->cs, [&]() {
            instance = self->app->NewMethodParamsInstance(*((Class*)pyClass)->miClass, methodName);
        });
        return (PyObject*)Instance_New(instance);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Application_NewInstance(Application *self, PyObject *args, PyObject *kwds)
{
    wchar_t* className = NULL;
    static char *kwlist[] = { "class_name", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "u", kwlist, &className))
        return NULL;

    try
    {
        std::shared_ptr<MI::Instance> instance;
        AllowThreads(&self->cs, [&]() {
            instance = self->app->NewInstance(className);
        });
        return (PyObject*)Instance_New(instance);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Application_NewInstanceFromClass(Application *self, PyObject *args, PyObject *kwds)
{
    wchar_t* className = NULL;
    PyObject* miClass = NULL;
    static char *kwlist[] = { "class_name", "mi_class", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uO", kwlist, &className, &miClass))
        return NULL;

    try
    {
        if (!PyObject_IsInstance(miClass, reinterpret_cast<PyObject*>(&ClassType)))
            throw MI::TypeConversionException(L"\"mi_class\" must have type Class");

        std::shared_ptr<MI::Instance> instance;
        AllowThreads(&self->cs, [&]() {
            instance = self->app->NewInstanceFromClass(className, *((Class*)miClass)->miClass);
        });
        return (PyObject*)Instance_New(instance);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Application_NewSerializer(Application* self, PyObject*)
{
    try
    {
        std::shared_ptr<MI::Serializer> serializer;
        AllowThreads(&self->cs, [&]() {
            serializer = self->app->NewSerializer();
        });
        return (PyObject*)Serializer_New(serializer);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Application_NewOperationOptions(Application* self, PyObject*)
{
    try
    {
        std::shared_ptr<MI::OperationOptions> operationOptions;
        AllowThreads(&self->cs, [&]() {
            operationOptions = self->app->NewOperationOptions();
        });
        return (PyObject*)OperationOptions_New(operationOptions);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Application_NewDestinationOptions(Application* self, PyObject*)
{
    try
    {
        std::shared_ptr<MI::DestinationOptions> destinationOptions;
        AllowThreads(&self->cs, [&]() {
            destinationOptions = self->app->NewDestinationOptions();
        });
        return (PyObject*)DestinationOptions_New(destinationOptions);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Application_Close(Application *self, PyObject*)
{
    try
    {
        AllowThreads(&self->cs, [&]() {
            self->app->Close();
        });
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Application_self(Application *self, PyObject*)
{
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject* Application_exit(Application* self, PyObject*)
{
    AllowThreads(&self->cs, [&]() {
        if (!self->app->IsClosed())
            self->app->Close();
    });
    Py_RETURN_NONE;
}

static PyMemberDef Application_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Application_methods[] = {
    { "create_session", (PyCFunction)Application_NewSession, METH_VARARGS | METH_KEYWORDS, "Creates a new session." },
    { "create_instance", (PyCFunction)Application_NewInstance, METH_VARARGS | METH_KEYWORDS, "Creates a new instance." },
    { "create_instance_from_class", (PyCFunction)Application_NewInstanceFromClass, METH_VARARGS | METH_KEYWORDS, "Creates a new instance from a class." },
    { "create_method_params", (PyCFunction)Application_NewMethodInboundParameters, METH_VARARGS | METH_KEYWORDS, "Creates a new __parameters instance with a method's inbound parameters." },
    { "create_serializer", (PyCFunction)Application_NewSerializer, METH_NOARGS, "Creates a serializer." },
    { "create_operation_options", (PyCFunction)Application_NewOperationOptions, METH_NOARGS, "Creates a new OperationObjects instance." },
    { "create_destination_options", (PyCFunction)Application_NewDestinationOptions, METH_NOARGS, "Creates a new DestinationOptions instance."},
    { "close", (PyCFunction)Application_Close, METH_NOARGS, "Closes the application." },
    { "__enter__", (PyCFunction)Application_self, METH_NOARGS, "" },
    { "__exit__",  (PyCFunction)Application_exit, METH_VARARGS, "" },
    { NULL }  /* Sentinel */
};

PyTypeObject ApplicationType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mi.Application",             /*tp_name*/
    sizeof(Application),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Application_dealloc, /*tp_dealloc*/
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
    "Application objects",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    Application_methods,             /* tp_methods */
    Application_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Application_init,      /* tp_init */
    0,                         /* tp_alloc */
    Application_new,                 /* tp_new */
};
