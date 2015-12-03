#include "stdafx.h"
#include "OperationOptions.h"
#include "PyMI.h"
#include "Utils.h"

#include <datetime.h>


static PyObject* OperationOptions_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	OperationOptions* self = NULL;
	self = (OperationOptions*)type->tp_alloc(type, 0);
	self->operationOptions = NULL;
	return (PyObject *)self;
}

static int OperationOptions_init(OperationOptions* self, PyObject* args, PyObject* kwds)
{
	PyErr_SetString(PyMIError, "An OperationOptions object cannot be allocated directly.");
	return -1;
}

static void OperationOptions_dealloc(OperationOptions* self)
{
	self->operationOptions = NULL;
	Py_TYPE(self)->tp_free((PyObject*)self);
}

OperationOptions* OperationOptions_New(std::shared_ptr<MI::OperationOptions> operationOptions)
{
	OperationOptions* obj = (OperationOptions*)OperationOptions_new(&OperationOptionsType, NULL, NULL);
	obj->operationOptions = operationOptions;
	return obj;
}

static PyObject* OperationOptions_Clone(OperationOptions *self, PyObject*)
{
	try
	{
		auto operationOptions = self->operationOptions->Clone();
		return (PyObject*)OperationOptions_New(operationOptions);
	}
	catch (std::exception& ex)
	{
		SetPyException(ex);
		return NULL;
	}
}

static PyObject* OperationOptions_GetTimeout(OperationOptions* self, PyObject* timeout)
{
	try
	{
		return PyDeltaFromMIInterval(self->operationOptions->GetTimeout());
	}
	catch (std::exception& ex)
	{
		SetPyException(ex);
		return NULL;
	}
}


static PyObject* OperationOptions_SetTimeout(OperationOptions* self, PyObject* timeout)
{
	PyDateTime_IMPORT;

	if (!PyDelta_Check(timeout))
	{
		PyErr_SetString(PyExc_TypeError, "parameter timeout must be of type datetime.timedelta");
		return NULL;
	}

	try
	{
		MI_Interval miTimeout;
		MIIntervalFromPyDelta(timeout, miTimeout);
		self->operationOptions->SetTimeout(miTimeout);
		Py_RETURN_NONE;
	}
	catch (std::exception& ex)
	{
		SetPyException(ex);
		return NULL;
	}
}

static PyMemberDef OperationOptions_members[] = {
	{ NULL }  /* Sentinel */
};

static PyMethodDef OperationOptions_methods[] = {
	{ "clone", (PyCFunction)OperationOptions_Clone, METH_NOARGS, "Clones the OperationOptions." },
	{ "get_timeout", (PyCFunction)OperationOptions_GetTimeout, METH_NOARGS, "Returns the timeout." },
	{ "set_timeout", (PyCFunction)OperationOptions_SetTimeout, METH_O, "Sets a timeout." },
	{ NULL }  /* Sentinel */
};

PyTypeObject OperationOptionsType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"mi.operationoptions",     /*tp_name*/
	sizeof(OperationOptions),  /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)OperationOptions_dealloc, /*tp_dealloc*/
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
	"OperationOptions objects",           /* tp_doc */
	0,		               /* tp_traverse */
	0,		               /* tp_clear */
	0,		               /* tp_richcompare */
	0,		               /* tp_weaklistoffset */
	0,		               /* tp_iter */
	0,		               /* tp_iternext */
	OperationOptions_methods,             /* tp_methods */
	OperationOptions_members,             /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)OperationOptions_init,    /* tp_init */
	0,                         /* tp_alloc */
	OperationOptions_new,                 /* tp_new */
};
