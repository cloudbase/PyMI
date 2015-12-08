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

bool PythonMICallbacks::WriteError(std::shared_ptr<MI::Operation> operation, std::shared_ptr<const MI::Instance> instance)
{
    return true;
}

void PythonMICallbacks::IndicationResult(std::shared_ptr<MI::Operation> operation, std::shared_ptr<const MI::Instance> instance,
    const std::wstring& bookmark, const std::wstring& machineID, bool moreResults, MI_Result resultCode,
    const std::wstring& errorString, std::shared_ptr<const MI::Instance> errorDetails)
{
    if (m_indicationResult)
    {
        PyObject* instanceObj = NULL;
        PyObject* errorDetailsObj = NULL;

        if (instance)
        {
            instanceObj = (PyObject*)Instance_New(std::const_pointer_cast<MI::Instance>(instance));
        }
        else
        {
            instanceObj = Py_None;
            Py_INCREF(Py_None);
        }

        if (errorDetails)
        {
            errorDetailsObj = (PyObject*)Instance_New(std::const_pointer_cast<MI::Instance>(errorDetails));
        }
        else
        {
            errorDetailsObj = Py_None;
            Py_INCREF(Py_None);
        }

        CallPythonCallback(m_indicationResult, "(OuuIIuO)", instanceObj, bookmark.c_str(), machineID.c_str(), moreResults ? 1 : 0,
            resultCode, errorString.c_str(), errorDetailsObj);

        Py_XDECREF(instanceObj);
        Py_XDECREF(errorDetailsObj);
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
