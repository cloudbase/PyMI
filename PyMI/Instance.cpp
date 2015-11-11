#include "stdafx.h"
#include "Instance.h"
#include "Class.h"
#include "Utils.h"


PyObject* Instance_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Instance* self = NULL;
    self = (Instance*)type->tp_alloc(type, 0);
    self->instance = NULL;
    return (PyObject *)self;
}

static void Instance_dealloc(Instance* self)
{
    if (self->instance)
    {
        delete self->instance;
        self->instance = NULL;
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject* Instance_subscript(Instance *self, PyObject *item)
{
    // TODO: size buffer based on actual size
    wchar_t w[1024];
    Py_ssize_t i;
    if (!GetIndexOrName(item, w, i))
        return NULL;

    const MI_Char* itemName;
    MI_Value itemValue;
    MI_Type itemType;
    MI_Uint32 itemFlags;

    if (i >= 0)
    {
        std::tie(itemName, itemValue, itemType, itemFlags) = (*self->instance)[i];
    }
    else
    {
        std::tie(itemValue, itemType, itemFlags) = (*self->instance)[w];
    }
    return MI2Py(itemValue, itemType, itemFlags);
}

static Py_ssize_t Instance_length(Instance *self)
{
    return self->instance->GetElementsCount();
}

static int Instance_ass_subscript(Instance* self, PyObject* item, PyObject* value)
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
    MI_Type miType = self->instance->GetElementType(w);

    Py2MI(value, miValue, miType);
    self->instance->SetElement(w, &miValue, miType);

    return 0;
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

static PyObject* Instance_GetClass(Instance *self, PyObject *args, PyObject *kwds)
{
    MI::Class* c = self->instance->GetClass();
    PyObject* pyClass = Class_new(&ClassType, NULL, NULL);
    //Py_INCREF(pyInstance);
    ((Class*)pyClass)->miClass = c;
    return pyClass;
}

static PyObject* Instance_GetClassName(Instance *self)
{
    std::wstring className = self->instance->GetClassName();
    return PyUnicode_FromWideChar(className.c_str(), className.length());
}

static PyMemberDef Instance_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Instance_methods[] = {
    { "__getitem__", (PyCFunction)Instance_subscript, METH_O | METH_COEXIST, "" },
    { "get_class_name", (PyCFunction)Instance_GetClassName, METH_NOARGS, "" },
    { "get_class", (PyCFunction)Instance_GetClass, METH_NOARGS, "" },
    { NULL }  /* Sentinel */
};

static PyMappingMethods Instance_as_mapping = {
    (lenfunc)Instance_length,
    (binaryfunc)Instance_subscript,
    (objobjargproc)Instance_ass_subscript
};

PyTypeObject InstanceType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
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
    0,                          /* tp_init */
    0,                         /* tp_alloc */
    Instance_new,                 /* tp_new */
};
