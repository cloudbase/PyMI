#include "stdafx.h"
#include "Operation.h"
#include "Instance.h"
#include "Class.h"
#include "Utils.h"
#include "PyMI.h"


static PyObject* Operation_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Operation* self = NULL;
    self = (Operation*)type->tp_alloc(type, 0);
    self->operation = NULL;
    ::InitializeCriticalSection(&self->cs);
    return (PyObject *)self;
}

static int Operation_init(Operation* self, PyObject* args, PyObject* kwds)
{
    PyErr_SetString(PyMIError, "An Operation object cannot be allocated directly.");
    return -1;
}

static void Operation_dealloc(Operation* self)
{
    AllowThreads(&self->cs, [&]() {
        self->operation = NULL;
    });
    ::DeleteCriticalSection(&self->cs);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* Operation_Cancel(Operation* self, PyObject*)
{
    try
    {
        AllowThreads(&self->cs, [&]() {
            self->operation->Cancel();
        });
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Operation_GetNextInstance(Operation* self, PyObject*)
{
    try
    {
        std::shared_ptr<MI::Instance> instance;
        AllowThreads(&self->cs, [&]() {
            instance = self->operation->GetNextInstance();
        });
        if (instance)
        {
            return (PyObject*)Instance_New(instance);
        }
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Operation_GetNextIndication(Operation* self, PyObject*)
{
    try
    {
        std::shared_ptr<MI::Instance> instance;
        AllowThreads(&self->cs, [&]() {
            instance = self->operation->GetNextIndication();
        });
        if (instance)
        {
            return (PyObject*)Instance_New(instance);
        }
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Operation_GetNextClass(Operation* self, PyObject*)
{
    try
    {
        std::shared_ptr<MI::Class> miClass;
        AllowThreads(&self->cs, [&]() {
            miClass = self->operation->GetNextClass();
        });
        if (miClass)
        {
            return (PyObject*)Class_New(miClass);
        }
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Operation_HasMoreResults(Operation* self, PyObject*)
{
    try
    {
        if (self->operation->HasMoreResults())
        {
            Py_RETURN_TRUE;
        }
        else
        {
            Py_RETURN_FALSE;
        }
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}


static PyObject* Operation_Close(Operation *self, PyObject*)
{
    try
    {
        self->operation->Close();
        Py_RETURN_NONE;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Operation_self(Operation *self, PyObject*)
{
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject* Operation_exit(Operation* self, PyObject*)
{
    AllowThreads(&self->cs, [&]() {
        if (!self->operation->IsClosed())
            self->operation->Close();
    });
    Py_RETURN_NONE;
}

Operation* Operation_New(std::shared_ptr<MI::Operation> operation)
{
    Operation* obj = (Operation*)Operation_new(&OperationType, NULL, NULL);
    obj->operation = operation;
    return obj;
}

static PyMemberDef Operation_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Operation_methods[] = {
    { "get_next_instance", (PyCFunction)Operation_GetNextInstance, METH_NOARGS, "Returns the next instance." },
    { "get_next_class", (PyCFunction)Operation_GetNextClass, METH_NOARGS, "Returns the next class." },
    { "get_next_indication", (PyCFunction)Operation_GetNextIndication, METH_NOARGS, "Returns the next result from a subscription." },
    { "has_more_results", (PyCFunction)Operation_HasMoreResults, METH_NOARGS, "Returns whether the current operation has more results." },
    { "cancel", (PyCFunction)Operation_Cancel, METH_NOARGS, "Cancels the operation." },
    { "close", (PyCFunction)Operation_Close, METH_NOARGS, "Closes the operation." },
    { "__enter__", (PyCFunction)Operation_self, METH_NOARGS, "" },
    { "__exit__",  (PyCFunction)Operation_exit, METH_VARARGS, "" },
    { NULL }  /* Sentinel */
};

PyTypeObject OperationType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mi.operation",             /*tp_name*/
    sizeof(Operation),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Operation_dealloc, /*tp_dealloc*/
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
    "Operation objects",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    Operation_methods,             /* tp_methods */
    Operation_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Operation_init,    /* tp_init */
    0,                         /* tp_alloc */
    Operation_new,                 /* tp_new */
};
