#include "stdafx.h"
#include "Application.h"
#include "Session.h"
#include "Instance.h"


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

static PyObject* Application_NewSession(Application *self, PyObject *args, PyObject *kwds)
{
    wchar_t* protocol = L"";
    wchar_t* computerName = L".";

    static char *kwlist[] = { "protocol", "computerName", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|uu", kwlist, &protocol, &computerName))
        return NULL;

    MI::Session* session = self->app->NewSession(protocol, computerName);
    PyObject* pySession = Session_new(&SessionType, NULL, NULL);
    //Py_INCREF(pyInstance);
    ((Session*)pySession)->session = session;
    return pySession;
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

static PyObject* Application_NewInstance(Application *self, PyObject *args, PyObject *kwds)
{
    wchar_t* className = NULL;
    static char *kwlist[] = { "className", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "u", kwlist, &className))
        return NULL;

    MI::Instance* instance = self->app->NewInstance(className);
    PyObject* pyInstance = Instance_new(&InstanceType, NULL, NULL);
    //Py_INCREF(pyInstance);
    ((Instance*)pyInstance)->instance = instance;
    return pyInstance;
}

static PyMemberDef Application_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Application_methods[] = {
    { "create_instance", (PyCFunction)Application_NewInstance, METH_KEYWORDS, "Creates a new instance." },
    { "create_session", (PyCFunction)Application_NewSession, METH_KEYWORDS, "Creates a new session." },
    { NULL }  /* Sentinel */
};

PyTypeObject ApplicationType = {
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
