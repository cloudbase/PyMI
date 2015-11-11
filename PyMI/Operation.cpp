#include "stdafx.h"
#include "Operation.h"
#include "Instance.h"


PyObject* Operation_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Operation* self = NULL;
    self = (Operation*)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static void Operation_dealloc(Operation* self)
{
    if (self->operation)
    {
        delete self->operation;
        self->operation = NULL;
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject* Operation_GetNextInstance(Operation* self)
{
    MI::Instance* instance = self->operation->GetNextInstance();
    if (instance)
    {
        PyObject* pyInstance = Instance_new(&InstanceType, NULL, NULL);
        //Py_INCREF(pyInstance);
        ((Instance*)pyInstance)->instance = instance;
        return pyInstance;
    }

    Py_RETURN_NONE;
}

static PyMemberDef Operation_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Operation_methods[] = {
    { "get_next_instance", (PyCFunction)Operation_GetNextInstance, METH_NOARGS, "Returns the next instance." },
    { NULL }  /* Sentinel */
};

PyTypeObject OperationType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
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
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Operation_methods,             /* tp_methods */
    Operation_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                          /* tp_init */
    0,                         /* tp_alloc */
    Operation_new,                 /* tp_new */
};
