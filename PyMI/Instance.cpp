#include "stdafx.h"
#include "Instance.h"
#include "Class.h"
#include "Utils.h"
#include "PyMI.h"


static PyObject* Instance_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Instance* self = NULL;
    self = (Instance*)type->tp_alloc(type, 0);
    self->instance = NULL;
    return (PyObject *)self;
}

static int Instance_init(Instance* self, PyObject* args, PyObject* kwds)
{
    PyErr_SetString(PyMIError, "An Instance object cannot be allocated directly.");
    return -1;
}

static void Instance_dealloc(Instance* self)
{
    if (self->instance)
    {
        delete self->instance;
        self->instance = NULL;
    }

    Py_TYPE(self)->tp_free((PyObject*)self);
}


MI::ValueElement GetElement(Instance *self, PyObject *item)
{
    // TODO: size buffer based on actual size
    wchar_t w[1024];
    Py_ssize_t i;
    if (!GetIndexOrName(item, w, i))
        throw MI::Exception(L"Cannot find element");

    MI::ValueElement element;
    if (i >= 0)
    {
        return (*self->instance)[(unsigned)i];
    }
    else
    {
        return (*self->instance)[w];
    }
}

static PyObject* Instance_subscript(Instance *self, PyObject *item)
{
    try
    {
        MI::ValueElement element = GetElement(self, item);
        return MI2Py(element.m_value, element.m_type, element.m_flags);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Instance_GetElement(Instance *self, PyObject *item)
{
    try
    {
        MI::ValueElement element = GetElement(self, item);
        PyObject* tuple = PyTuple_New(3);
        PyTuple_SetItem(tuple, 0, PyUnicode_FromWideChar(element.m_name.c_str(), element.m_name.length()));
#ifdef IS_PY3K
        PyTuple_SetItem(tuple, 1, PyLong_FromLong(element.m_type));
#else
        PyTuple_SetItem(tuple, 1, PyInt_FromLong(element.m_type));
#endif
        PyTuple_SetItem(tuple, 2, MI2Py(element.m_value, element.m_type, element.m_flags));
        return tuple;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static Py_ssize_t Instance_length(Instance *self)
{
    try
    {
        return self->instance->GetElementsCount();
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return -1;
    }
}

static int Instance_ass_subscript(Instance* self, PyObject* item, PyObject* value)
{
    try
    {
        // TODO: size buffer based on actual size
        wchar_t w[1024];
        w[0] = NULL;
        Py_ssize_t i = 0;
        if (!GetIndexOrName(item, w, i))
            return NULL;

        bool addElement = false;

        MI_Value miValue;
        MI_UNREFERENCED_PARAMETER(&miValue);
        MI_Type miType;
        if (i >= 0)
        {
            miType = self->instance->GetElementType((unsigned)i);
        }
        else
        {
            miType = self->instance->GetElementType(w);
        }

        Py2MI(value, miValue, miType);

        if (i >= 0)
        {
            self->instance->SetElement((unsigned)i, value == Py_None? NULL : &miValue, miType);
        }
        else
        {
            self->instance->SetElement(w, value == Py_None ? NULL : &miValue, miType);
        }

        return 0;
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return -1;
    }
}

static PyObject* Instance_getattro(Instance *self, PyObject* name)
{
    PyObject* attr = PyObject_GenericGetAttr((PyObject*)self, name);
    if (attr)
    {
        return attr;
    }

    return Instance_subscript(self, name);
}

static int Instance_setattro(Instance *self, PyObject* name, PyObject* value)
{
    return -1;
}

static PyObject* Instance_GetClass(Instance *self, PyObject*)
{
    try
    {
        MI::Class* c = self->instance->GetClass();
        return (PyObject*)Class_New(c);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Instance_Clone(Instance *self, PyObject*)
{
    try
    {
        MI::Instance* instance = self->instance->Clone();
        return (PyObject*)Instance_New(instance);
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Instance_GetPath(Instance *self, PyObject*)
{
    try
    {
        std::wstring path = self->instance->GetPath();
        return PyUnicode_FromWideChar(path.c_str(), path.length());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Instance_GetClassName(Instance *self, PyObject*)
{
    try
    {
        std::wstring className = self->instance->GetClassName();
        return PyUnicode_FromWideChar(className.c_str(), className.length());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

Instance* Instance_New(MI::Instance* instance)
{
    Instance* obj = (Instance*)Instance_new(&InstanceType, NULL, NULL);
    obj->instance = instance;
    return obj;
}

static PyMemberDef Instance_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Instance_methods[] = {
    { "__getitem__", (PyCFunction)Instance_subscript, METH_O | METH_COEXIST, "" },
    { "get_element", (PyCFunction)Instance_GetElement, METH_O, "Returns an element by either index or name" },
    { "get_path", (PyCFunction)Instance_GetPath, METH_NOARGS, "" },
    { "get_class_name", (PyCFunction)Instance_GetClassName, METH_NOARGS, "" },
    { "get_class", (PyCFunction)Instance_GetClass, METH_NOARGS, "" },
    { "clone", (PyCFunction)Instance_Clone, METH_NOARGS, "Clones this instance." },
    { NULL }  /* Sentinel */
};

static PyMappingMethods Instance_as_mapping = {
    (lenfunc)Instance_length,
    (binaryfunc)Instance_subscript,
    (objobjargproc)Instance_ass_subscript
};

PyTypeObject InstanceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mi.Instance",             /*tp_name*/
    sizeof(Instance),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Instance_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &Instance_as_mapping,      /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    (getattrofunc)Instance_getattro, /*tp_getattro*/
    (setattrofunc)Instance_setattro, /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Instance objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Instance_methods,             /* tp_methods */
    Instance_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Instance_init,    /* tp_init */
    0,                         /* tp_alloc */
    Instance_new,                 /* tp_new */
};
