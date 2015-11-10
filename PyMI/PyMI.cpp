// PyMI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <MI++.h>
#include "Utils.h"
#include <datetime.h>

static PyObject *PyMIError;

typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        MI::Application* app;
} Application;

static PyObject* Application_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Application* self = NULL;
    self = (Application*)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int Application_init(Application *self, PyObject *args, PyObject *kwds)
{
    wchar_t* appId = L"";
    static char *kwlist[] = { "appId", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|u", kwlist, &appId))
        return -1;

    self->app = new MI::Application(appId);
    return 0;
}

static void Application_dealloc(Application* self)
{
    if (self->app)
    {
        delete self->app;
        self->app = NULL;
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyMemberDef Application_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Application_methods[] = {
    { NULL }  /* Sentinel */
};

static PyTypeObject ApplicationType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
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
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
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

typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        PyObject* app;
        MI::Session* session;
} Session;

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

static PyMemberDef Session_members[] = {
    { "app", T_OBJECT_EX, offsetof(Session, app), 0, "Application" },
    { NULL }  /* Sentinel */
};

static PyMethodDef Session_methods[] = {
    { NULL }  /* Sentinel */
};

static PyTypeObject SessionType = {
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


typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        MI::Instance instance;
} Instance;

static PyObject* Instance_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Instance* self = NULL;
    self = (Instance*)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static void Instance_dealloc(Instance* self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject* Instance_subscript(Instance *self, PyObject *item)
{
    // TODO: size buffer based on actual size
    wchar_t w[1024];
    w[0] = NULL;
    Py_ssize_t i = -1;

    if (PyString_Check(item))
    {
        char* s = PyString_AsString(item);
        ::MultiByteToWideChar(CP_ACP, 0, s, -1, w, 1024);
    }
    else if (PyUnicode_Check(item))
    {
        if (PyUnicode_AsWideChar((PyUnicodeObject*)item, w, 1024) < 0)
            return NULL;
    }
    else if (PyIndex_Check(item))
    {
        i = PyNumber_AsSsize_t(item, PyExc_IndexError);
        if (i == -1 && PyErr_Occurred())
            return NULL;
    }
    else
        return NULL;

    const MI_Char* itemName;
    MI_Value itemValue;
    MI_Type itemType;
    MI_Uint32 itemFlags;

    if (i >= 0)
    {
        std::tie(itemName, itemValue, itemType, itemFlags) = self->instance[i];
    }
    else
    {
        std::tie(itemValue, itemType, itemFlags) = self->instance[w];
    }
    return MI2Py(itemValue, itemType, itemFlags);
}

static Py_ssize_t Instance_length(Instance *self)
{
    return self->instance.GetElementsCount();
}

static int Instance_ass_subscript(Instance* self, PyObject* item, PyObject* value)
{
    return -1;
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

static PyObject* Instance_GetClassName(Instance *self)
{
    std::wstring className = self->instance.GetClassName();
    return PyUnicode_FromWideChar(className.c_str(), className.length());
}

static PyMemberDef Instance_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Instance_methods[] = {
    { "__getitem__", (PyCFunction)Instance_subscript, METH_O | METH_COEXIST, "" },
    { "get_class_name", (PyCFunction)Instance_GetClassName, METH_NOARGS, "" },
    { NULL }  /* Sentinel */
};

static PyMappingMethods Instance_as_mapping = {
    (lenfunc)Instance_length,
    (binaryfunc)Instance_subscript,
    (objobjargproc)Instance_ass_subscript
};

static PyTypeObject InstanceType = {
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
    0,                         /*tp_setattro*/
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


typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        PyObject* session;
        MI::Query* query;
} Query;

static PyObject* Query_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Query* self = NULL;
    self = (Query*)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int Query_init(Query *self, PyObject *args, PyObject *kwds)
{
    PyObject* session = NULL;
    wchar_t* ns = NULL;
    wchar_t* query = NULL;
    wchar_t* dialect = L"WQL";

    static char *kwlist[] = { "session", "ns", "query", "dialect", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ouu|u", kwlist, &session, &ns, &query, &dialect))
        return -1;

    if (!PyObject_IsInstance(session, reinterpret_cast<PyObject*>(&SessionType)))
        return -1;

    PyObject* tmp = self->session;
    Py_INCREF(session);
    self->session = session;
    Py_XDECREF(tmp);

    self->query = new MI::Query(*((Session*)session)->session, ns, query, dialect);
    return 0;
}

static void Query_dealloc(Query* self)
{
    if (self->query)
    {
        delete self->query;
        self->query = NULL;
    }

    if (self->session)
    {
        Py_XDECREF(self->session);
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject* Query_GetNextInstance(Query* self)
{
    MI::Instance instance = self->query->GetNextInstance();
    if (instance)
    {
        PyObject* pyInstance = Instance_new(&InstanceType, NULL, NULL);
        Py_INCREF(pyInstance);
        ((Instance*)pyInstance)->instance = instance;
        return pyInstance;
    }

    Py_RETURN_NONE;
}

static PyMemberDef Query_members[] = {
    { "session", T_OBJECT_EX, offsetof(Query, session), 0, "Session" },
    { NULL }  /* Sentinel */
};

static PyMethodDef Query_methods[] = {
    { "get_next_instance", (PyCFunction)Query_GetNextInstance, METH_NOARGS, "Returns the next instance." },
    { NULL }  /* Sentinel */
};

static PyTypeObject QueryType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "mi.Query",             /*tp_name*/
    sizeof(Query),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Query_dealloc, /*tp_dealloc*/
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
    "Query objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Query_methods,             /* tp_methods */
    Query_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Query_init,      /* tp_init */
    0,                         /* tp_alloc */
    Query_new,                 /* tp_new */
};

static PyMethodDef mi_methods[] = {
    { NULL, NULL, 0, NULL }  /* Sentinel */
};

PyMODINIT_FUNC initmi(void)
{
    PyDateTime_IMPORT;

    PyObject* m = NULL;

    ApplicationType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ApplicationType) < 0)
        return;

    SessionType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&SessionType) < 0)
        return;

    InstanceType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&InstanceType) < 0)
        return;

    QueryType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&QueryType) < 0)
        return;

    m = Py_InitModule3("mi", mi_methods, "MI module.");
    if (m == NULL)
        return;

    Py_INCREF(&ApplicationType);
    PyModule_AddObject(m, "Application", (PyObject*)&ApplicationType);

    Py_INCREF(&SessionType);
    PyModule_AddObject(m, "Session", (PyObject*)&SessionType);

    Py_INCREF(&InstanceType);
    PyModule_AddObject(m, "Instance", (PyObject*)&InstanceType);

    Py_INCREF(&QueryType);
    PyModule_AddObject(m, "Query", (PyObject*)&QueryType);

    /*
    m = Py_InitModule("mi", PyMIMethods);
    if (m == NULL)
        return;

    PyMIError = PyErr_NewException("PyMI.error", NULL, NULL);
    Py_INCREF(PyMIError);
    PyModule_AddObject(m, "error", PyMIError);
    */
}
