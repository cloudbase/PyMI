#include "stdafx.h"
#include "Session.h"
#include "Application.h"
#include "Operation.h"
#include "Instance.h"
#include "Class.h"
#include "Callbacks.h"
#include "OperationOptions.h"
#include "Utils.h"
#include "PyMI.h"


static PyObject* Session_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Session* self = NULL;
    self = (Session*)type->tp_alloc(type, 0);
    self->session = NULL;
    self->operationCallbacks = std::make_shared<std::vector<std::shared_ptr<MI::Callbacks>>>();
    return (PyObject *)self;
}

static int Session_init(Session* self, PyObject* args, PyObject* kwds)
{
    PyErr_SetString(PyMIError, "Please use Application.create_session to allocate a Session object.");
    return -1;
}

static void Session_dealloc(Session* self)
{
    AllowThreads([&]() {
        self->session = NULL;
    });
    self->operationCallbacks = NULL;
    Py_TYPE(self)->tp_free((PyObject*)self);
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
        std::shared_ptr<MI::Operation> op;
        AllowThreads([&]() {
            op = self->session->ExecQuery(ns, query, dialect);
        });
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

    static char *kwlist[] = { "ns", "instance", "assoc_class", "result_class", "role", "result_role", "keys_only", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uO|uuuuO", kwlist, &ns, &instance, &assocClass, &resultClass, &role, &resultRole, &keysOnlyObj))
        return NULL;

    if (!PyObject_IsInstance(instance, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    bool keysOnly = keysOnlyObj && PyObject_IsTrue(keysOnlyObj);

    try
    {
        std::shared_ptr<MI::Operation> op;
        AllowThreads([&]() {
            op = self->session->GetAssociators(ns, *((Instance*)instance)->instance, assocClass, resultClass, role, resultRole, keysOnly);
        });
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
        AllowThreads([&]()
        {
            self->session->Close();
        });
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
    AllowThreads([&]()
    {
        if (!self->session->IsClosed())
            self->session->Close();
    });

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
        AllowThreads([&]() {
            self->session->CreateInstance(ns, *((Instance*)instance)->instance);
        });
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
        AllowThreads([&]() {
            self->session->ModifyInstance(ns, *((Instance*)instance)->instance);
        });
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
        AllowThreads([&]() {
            self->session->DeleteInstance(ns, *((Instance*)instance)->instance);
        });
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

    static char *kwlist[] = { "ns", "class_name", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uu", kwlist, &ns, &className))
        return NULL;

    try
    {
        std::shared_ptr<MI::Operation> op;
        AllowThreads([&]() {
            op = self->session->GetClass(ns, className);
        });
        return (PyObject*)Operation_New(op);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_GetInstance(Session *self, PyObject *args, PyObject *kwds)
{
    wchar_t* ns = NULL;
    PyObject* keyInstance = NULL;

    static char *kwlist[] = { "ns", "key_instance", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uO", kwlist, &ns, &keyInstance))
        return NULL;

    if (!PyObject_IsInstance(keyInstance, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    try
    {
        std::shared_ptr<MI::Operation> op;
        AllowThreads([&]() {
            op = self->session->GetInstance(ns, *((Instance*)keyInstance)->instance);
        });
        return (PyObject*)Operation_New(op);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_Subscribe(Session *self, PyObject *args, PyObject *kwds)
{
    wchar_t* ns = NULL;
    wchar_t* query = NULL;
    PyObject* indicationResultCallback = NULL;
    PyObject* operationOptions = NULL;
    wchar_t* dialect = L"WQL";

    static char *kwlist[] = { "ns", "query", "indication_result", "operation_options", "dialect", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "uu|OOu", kwlist, &ns, &query, &indicationResultCallback, &operationOptions, &dialect))
        return NULL;
    if (!CheckPyNone(indicationResultCallback) && !PyCallable_Check(indicationResultCallback))
    {
        PyErr_SetString(PyExc_TypeError, "parameter indication_result must be callable");
        return NULL;
    }
    if (!CheckPyNone(operationOptions) && !PyObject_IsInstance(operationOptions, reinterpret_cast<PyObject*>(&OperationOptionsType)))
    {
        PyErr_SetString(PyExc_TypeError, "parameter operation_options must be of type OperationOptions");
        return NULL;
    }

    auto callbacks = !CheckPyNone(indicationResultCallback) ? std::make_shared<PythonMICallbacks>(indicationResultCallback) : NULL;

    try
    {
        std::shared_ptr<MI::Operation> op;
        AllowThreads([&]() {
            op = self->session->Subscribe(ns, query, callbacks,
                !CheckPyNone(operationOptions) ? ((OperationOptions*)operationOptions)->operationOptions : NULL,
                dialect);
        });
        PyObject* obj = (PyObject*)Operation_New(op);
        if (callbacks)
        {
            self->operationCallbacks->push_back(callbacks);
        }
        return obj;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Session_InvokeMethod(Session *self, PyObject *args, PyObject *kwds)
{
    PyObject* target = NULL;
    wchar_t* methodName = NULL;
    PyObject* inboundParams = NULL;

    static char *kwlist[] = { "target", "method_name", "inbound_params", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ou|O", kwlist, &target, &methodName, &inboundParams))
        return NULL;

    try
    {
        if (!CheckPyNone(inboundParams) && !PyObject_IsInstance(inboundParams, reinterpret_cast<PyObject*>(&InstanceType)))
            throw MI::TypeConversionException(L"\"inbound_params\" must have type Instance");

        std::shared_ptr<MI::Operation> op;
        if (PyObject_IsInstance(target, reinterpret_cast<PyObject*>(&InstanceType)))
        {
            AllowThreads([&]() {
                op = self->session->InvokeMethod(*((Instance*)target)->instance, methodName,
                    !CheckPyNone(inboundParams) ? ((Instance*)inboundParams)->instance : NULL);
            });
        }
        else if (PyObject_IsInstance(target, reinterpret_cast<PyObject*>(&ClassType)))
        {
            AllowThreads([&]() {
                auto miClass = ((Class*)target)->miClass;
                op = self->session->InvokeMethod(miClass->GetNameSpace(), miClass->GetClassName(), methodName,
                    !CheckPyNone(inboundParams) ? ((Instance*)inboundParams)->instance : NULL);
            });
        }
        else
        {
            throw MI::TypeConversionException(L"\"target\" must have type Instance or Class");
        }

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

Session* Session_New(std::shared_ptr<MI::Session> session)
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
    { "get_instance", (PyCFunction)Session_GetInstance, METH_VARARGS | METH_KEYWORDS, "Retrieves an instance." },
    { "subscribe", (PyCFunction)Session_Subscribe, METH_VARARGS | METH_KEYWORDS, "Subscribes to events." },
    { "close", (PyCFunction)Session_Close, METH_NOARGS, "Closes the session." },
    { "__enter__", (PyCFunction)Session_self, METH_NOARGS, "" },
    { "__exit__",  (PyCFunction)Session_exit, METH_VARARGS, "" },
    { NULL }  /* Sentinel */
};

PyTypeObject SessionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
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
