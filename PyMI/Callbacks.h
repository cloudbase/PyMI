#pragma once

#include <Python.h>
#include <MI++.h>

class PythonMICallbacks : public MI::Callbacks
{
private:
    PyObject* m_indicationResult = NULL;
public:
    PythonMICallbacks(PyObject* indicationResult);
    virtual bool WriteError(MI::Operation& operation, const MI::Instance& instance);
    void IndicationResult(MI::Operation& operation, const MI::Instance* instance, const std::wstring& bookmark,
        const std::wstring& machineID, bool moreResults, MI_Result resultCode,
        const std::wstring& errorString, const MI::Instance* errorDetails);
    ~PythonMICallbacks();
};
