#include "stdafx.h"
#include "Session.h"
#include "Application.h"
#include "Operation.h"
#include "Class.h"
#include "Instance.h"


static PyObject* Session_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Session* self = NULL;
    self = (Session*)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int Session_init(Session *self, PyObject *args, PyObject *kwds)
{
    PyObject* app = NULL;
    wchar_t* protocol = L"";
    wchar_t* computerName = L".";

    static char *kwlist[] = { "app", "protocol", "computerName", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|uu", kwlist, &app, &protocol, &computerName))
        return -1;

    if (!PyObject_IsInstance(app, reinterpret_cast<PyObject*>(&ApplicationType)))
        return -1;

    PyObject* tmp = self->app;
    Py_INCREF(app);
    self->app = app;
    Py_XDECREF(tmp);

    if (self->session)
    {
        delete self->session;
        self->session = NULL;
    }

    self->session = new MI::Session(*((Application*)app)->app, protocol, computerName);
    return 0;
}

static void Session_dealloc(Session* self)
{
    if (self->session)
    {
        delete self->session;
        self->session = NULL;
    }

    if (self->app)
    {
        Py_XDECREF(self->app);
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject* Session_ExecQuery(Session *self, PyObject *args, PyObject *kwds)
{
    wchar_t* ns = NULL;
    wchar_t* query = NULL;
    wchar_t* dialect = L"WQL";

    static char *kwlist[] = { "ns", "query", "dialect", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uu|u", kwlist, &ns, &query, &dialect))
        return NULL;

    MI::Operation* op = self->session->ExecQuery(ns, query, dialect);

    PyObject* pyOp = Operation_new(&OperationType, NULL, NULL);
    //Py_INCREF(pyOp);
    ((Operation*)pyOp)->operation = op;
    return pyOp;
}

static PyObject* Session_GetClass(Session *self, PyObject *args, PyObject *kwds)
{
    wchar_t* ns = NULL;
    wchar_t* className = NULL;

    static char *kwlist[] = { "ns", "className", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uu", kwlist, &ns, &className))
        return NULL;

    MI::Class* c = self->session->GetClass(ns, className);
    PyObject* pyClass = Class_new(&ClassType, NULL, NULL);
    //Py_INCREF(pyInstance);
    ((Class*)pyClass)->m_class = c;
    return pyClass;
}

static PyObject* Session_InvokeMethod(Session *self, PyObject *args, PyObject *kwds)
{
    PyObject* instance = NULL;
    wchar_t* methodName = NULL;
    PyObject* inboundParams = NULL;

    static char *kwlist[] = { "instance", "methodName", "inboundParams", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ou|O", kwlist, &instance, &methodName, &inboundParams))
        return NULL;

    if (!PyObject_IsInstance(instance, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    if (inboundParams && !PyObject_IsInstance(inboundParams, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    MI::Instance* result = self->session->InvokeMethod(*((Instance*)instance)->instance, methodName, inboundParams ? ((Instance*)inboundParams)->instance : NULL);
    if (result)
    {
        PyObject* pyInstance = Instance_new(&InstanceType, NULL, NULL);
        //Py_INCREF(pyInstance);
        ((Instance*)pyInstance)->instance = result;
        return pyInstance;
    }
    Py_RETURN_NONE;
}

static PyMemberDef Session_members[] = {
    { "app", T_OBJECT_EX, offsetof(Session, app), 0, "Application" },
    { NULL }  /* Sentinel */
};

static PyMethodDef Session_methods[] = {
    { "exec_query", (PyCFunction)Session_ExecQuery, METH_KEYWORDS, "Executes a query." },
    { "invoke_method", (PyCFunction)Session_InvokeMethod, METH_KEYWORDS, "invokes a method." },
    { "get_class", (PyCFunction)Session_GetClass, METH_KEYWORDS, "Gets a class." },
    { NULL }  /* Sentinel */
};

PyTypeObject SessionType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "mi.Session",             /*tp_name*/
    sizeof(Session),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Session_dealloc, /*tp_dealloc*/
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
    "Session objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Session_methods,             /* tp_methods */
    Session_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Session_init,      /* tp_init */
    0,                         /* tp_alloc */
    Session_new,                 /* tp_new */
};
