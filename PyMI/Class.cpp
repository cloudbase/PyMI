#include "stdafx.h"
#include "Class.h"
#include "Utils.h"
#include "PyMI.h"


static PyObject* Class_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Class* self = NULL;
    self = (Class*)type->tp_alloc(type, 0);
    self->miClass = NULL;
    ::InitializeCriticalSection(&self->cs);
    return (PyObject *)self;
}

static int Class_init(Class* self, PyObject* args, PyObject* kwds)
{
    PyErr_SetString(PyMIError, "A class object cannot be allocated directly.");
    return -1;
}

static void Class_dealloc(Class* self)
{
    AllowThreads(&self->cs, [&]() {
        self->miClass = NULL;
    });
    ::DeleteCriticalSection(&self->cs);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* Class_subscript(Class *self, PyObject *item)
{
    try
    {
        std::wstring name;
        Py_ssize_t i;
        GetIndexOrName(item, name, i);

        std::shared_ptr<MI::ClassElement> element;
        AllowThreads(&self->cs, [&]() {
            if (i >= 0)
            {
                element = (*self->miClass)[(unsigned)i];
            }
            else
            {
                element = (*self->miClass)[name];
            }
        });
        return MI2Py(element->m_value, element->m_type, element->m_flags);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static Py_ssize_t Class_length(Class *self)
{
    try
    {
        Py_ssize_t l = 0;
        AllowThreads(&self->cs, [&]() {
            l = self->miClass->GetElementsCount();
        });
        return l;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return -1;
    }
}

static PyObject* Class_getattro(Class *self, PyObject* name)
{
    PyObject* attr = PyObject_GenericGetAttr((PyObject*)self, name);
    if (attr)
    {
        return attr;
    }

    return Class_subscript(self, name);
}

Class* Class_New(std::shared_ptr<MI::Class> miClass)
{
    Class* obj = (Class*)Class_new(&ClassType, NULL, NULL);
    obj->miClass = miClass;
    return obj;
}

static PyObject* Class_Clone(Class* self, PyObject*)
{
    try
    {
        std::shared_ptr<MI::Class> miClass;
        AllowThreads(&self->cs, [&]() {
            miClass = self->miClass->Clone();
        });
        return (PyObject*)Class_New(miClass);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Class_GetClassName(Class* self, PyObject*)
{
    try
    {
        std::wstring& className = self->miClass->GetClassName();
        return PyUnicode_FromWideChar(className.c_str(), className.length());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Class_GetNameSpace(Class* self, PyObject*)
{
    try
    {
        std::wstring& nameSpace = self->miClass->GetNameSpace();
        return PyUnicode_FromWideChar(nameSpace.c_str(), nameSpace.length());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Class_GetServerName(Class* self, PyObject*)
{
    try
    {
        std::wstring& serverName = self->miClass->GetServerName();
        return PyUnicode_FromWideChar(serverName.c_str(), serverName.length());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyMemberDef Class_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Class_methods[] = {
    { "get_class_name", (PyCFunction)Class_GetClassName, METH_NOARGS, "" },
    { "get_namespace", (PyCFunction)Class_GetNameSpace, METH_NOARGS, "" },
    { "get_server_name", (PyCFunction)Class_GetServerName, METH_NOARGS, "" },
    { "__getitem__", (PyCFunction)Class_subscript, METH_O | METH_COEXIST, "" },
    { "clone", (PyCFunction)Class_Clone, METH_NOARGS, "Clones this class." },
    { NULL }  /* Sentinel */
};

static PyMappingMethods Class_as_mapping = {
    (lenfunc)Class_length,
    (binaryfunc)Class_subscript,
    NULL
};

PyTypeObject ClassType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mi.Class",             /*tp_name*/
    sizeof(Class),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Class_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &Class_as_mapping,      /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    (getattrofunc)Class_getattro, /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Class objects",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    Class_methods,             /* tp_methods */
    Class_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Class_init,    /* tp_init */
    0,                         /* tp_alloc */
    Class_new,                 /* tp_new */
};

