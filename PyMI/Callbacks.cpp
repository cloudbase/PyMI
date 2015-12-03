#include "stdafx.h"
#include "Callbacks.h"
#include "Instance.h"
#include "Utils.h"
#include "PyMI.h"


PythonMICallbacks::PythonMICallbacks(PyObject* indicationResult) : m_indicationResult(indicationResult)
{
    if (m_indicationResult)
    {
        Py_XINCREF(m_indicationResult);
    }
}

bool PythonMICallbacks::WriteError(MI::Operation& operation, const MI::Instance& instance)
{
    return true;
}

void PythonMICallbacks::IndicationResult(MI::Operation& operation, const MI::Instance* instance, const std::wstring& bookmark,
    const std::wstring& machineID, bool moreResults, MI_Result resultCode,
    const std::wstring& errorString, const MI::Instance* errorDetails)
{
    if (m_indicationResult)
    {
        PyObject* instanceObj = NULL;
        PyObject* errorDetailsObj = NULL;

        if (instance)
        {
            instanceObj = (PyObject*)Instance_New((MI::Instance *)instance, false);
        }
        else
        {
            instanceObj = Py_None;
            Py_INCREF(Py_None);
        }

        if (errorDetails)
        {
            errorDetailsObj = (PyObject*)Instance_New((MI::Instance *)errorDetails, false);
        }
        else
        {
            errorDetailsObj = Py_None;
            Py_INCREF(Py_None);
        }

        CallPythonCallback(m_indicationResult, "(OuuIIuO)", instanceObj, bookmark.c_str(), machineID.c_str(), moreResults ? 1 : 0,
            resultCode, errorString.c_str(), errorDetailsObj);

        Py_DECREF(instanceObj);
        Py_DECREF(errorDetailsObj);
    }
}

PythonMICallbacks::~PythonMICallbacks()
{
    if (m_indicationResult)
    {
        Py_XDECREF(m_indicationResult);
        m_indicationResult = NULL;
    }
}
