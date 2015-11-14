#include "stdafx.h"
#include "Session.h"
#include "Application.h"
#include "Operation.h"
#include "Instance.h"
#include "Utils.h"
#include "PyMI.h"

static PyObject* Session_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Session* self = NULL;
    self = (Session*)type->tp_alloc(type, 0);
    self->session = NULL;
    return (PyObject *)self;
}

static int Session_init(Session* self, PyObject* args, PyObject* kwds)
{
    PyErr_SetString(PyMIError, "Please use Application.create_session to allocate a Session object.");
    return -1;
}

static void Session_dealloc(Session* self)
{
    if (self->session)
    {
        delete self->session;
        self->session = NULL;
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

    try
    {
        MI::Operation* op = self->session->ExecQuery(ns, query, dialect);
        return (PyObject*)Operation_New(op);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_GetAssociators(Session *self, PyObject *args, PyObject *kwds)
{
    PyObject* instance = NULL;
    wchar_t* ns = NULL;
    wchar_t* assocClass = L"";
    wchar_t* resultClass = L"";
    wchar_t* role = L"";
    wchar_t* resultRole = L"";
    PyObject* keysOnlyObj = NULL;

    static char *kwlist[] = { "ns", "instance", "assocClass", "resultClass", "role", "resultRole", "keysOnly", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uO|uuuuO", kwlist, &ns, &instance, &assocClass, &resultClass, &role, &resultRole, &keysOnlyObj))
        return NULL;

    if (!PyObject_IsInstance(instance, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    bool keysOnly = keysOnlyObj && PyObject_IsTrue(keysOnlyObj);

    try
    {
        MI::Operation* op = self->session->GetAssociators(ns, *((Instance*)instance)->instance, assocClass, resultClass, role, resultRole, keysOnly);
        return (PyObject*)Operation_New(op);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_Close(Session *self, PyObject*)
{
    try
    {
        self->session->Close();
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_self(Operation *self, PyObject*)
{
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject* Session_exit(Session* self, PyObject*)
{
    if (!self->session->IsClosed())
        self->session->Close();
    Py_RETURN_NONE;
}

static PyObject* Session_CreateInstance(Session *self, PyObject *args, PyObject *kwds)
{
    wchar_t* ns = NULL;
    PyObject* instance = NULL;

    static char *kwlist[] = { "ns", "instance", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uO", kwlist, &ns, &instance))
        return NULL;

    if (!PyObject_IsInstance(instance, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    try
    {
        self->session->CreateInstance(ns, *((Instance*)instance)->instance);
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_ModifyInstance(Session *self, PyObject *args, PyObject *kwds)
{
    wchar_t* ns = NULL;
    PyObject* instance = NULL;

    static char *kwlist[] = { "ns", "instance", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uO", kwlist, &ns, &instance))
        return NULL;

    if (!PyObject_IsInstance(instance, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    try
    {
        self->session->ModifyInstance(ns, *((Instance*)instance)->instance);
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_DeleteInstance(Session *self, PyObject *args, PyObject *kwds)
{
    wchar_t* ns = NULL;
    PyObject* instance = NULL;

    static char *kwlist[] = { "ns", "instance", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uO", kwlist, &ns, &instance))
        return NULL;

    if (!PyObject_IsInstance(instance, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    try
    {
        self->session->DeleteInstance(ns, *((Instance*)instance)->instance);
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_GetClass(Session *self, PyObject *args, PyObject *kwds)
{
    wchar_t* ns = NULL;
    wchar_t* className = NULL;

    static char *kwlist[] = { "ns", "className", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uu", kwlist, &ns, &className))
        return NULL;

    try
    {
        MI::Operation* op = self->session->GetClass(ns, className);
        return (PyObject*)Operation_New(op);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
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

    try
    {
        MI::Operation* op = self->session->InvokeMethod(*((Instance*)instance)->instance, methodName, inboundParams ? ((Instance*)inboundParams)->instance : NULL);
        if (op)
        {
            return (PyObject*)Operation_New(op);
        }
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

Session* Session_New(MI::Session* session)
{
    Session* obj = (Session*)Session_new(&SessionType, NULL, NULL);
    obj->session = session;
    return obj;
}

static PyMemberDef Session_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Session_methods[] = {
    { "exec_query", (PyCFunction)Session_ExecQuery, METH_VARARGS | METH_KEYWORDS, "Executes a query." },
    { "invoke_method", (PyCFunction)Session_InvokeMethod, METH_VARARGS | METH_KEYWORDS, "Invokes a method." },
    { "get_associators", (PyCFunction)Session_GetAssociators, METH_VARARGS | METH_KEYWORDS, "Retrieves the associators of an instance." },
    { "get_class", (PyCFunction)Session_GetClass, METH_VARARGS | METH_KEYWORDS, "Gets a class." },
    { "create_instance", (PyCFunction)Session_CreateInstance, METH_VARARGS | METH_KEYWORDS, "Creates an instance." },
    { "modify_instance", (PyCFunction)Session_ModifyInstance, METH_VARARGS | METH_KEYWORDS, "Modifies an instance." },
    { "delete_instance", (PyCFunction)Session_DeleteInstance, METH_VARARGS | METH_KEYWORDS, "Deletes an instance." },
    { "close", (PyCFunction)Session_Close, METH_NOARGS, "Closes the session." },
    { "__enter__", (PyCFunction)Session_self, METH_NOARGS, "" },
    { "__exit__",  (PyCFunction)Session_exit, METH_VARARGS, "" },
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
    (initproc)Session_init,    /* tp_init */
    0,                         /* tp_alloc */
    Session_new,                 /* tp_new */
};
