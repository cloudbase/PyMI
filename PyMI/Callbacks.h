#pragma once

#include <Python.h>
#include <MI++.h>
#include <memory>

class PythonMICallbacks : public MI::Callbacks
{
private:
    PyObject* m_indicationResult = NULL;
public:
    PythonMICallbacks(PyObject* indicationResult);
    bool WriteError(std::shared_ptr<MI::Operation> operation, std::shared_ptr<const MI::Instance> instance);
    void IndicationResult(std::shared_ptr<MI::Operation> operation, std::shared_ptr<const MI::Instance> instance,
        const std::wstring& bookmark, const std::wstring& machineID, bool moreResults, MI_Result resultCode,
        const std::wstring& errorString, std::shared_ptr<const MI::Instance> errorDetails);
    ~PythonMICallbacks();
};
