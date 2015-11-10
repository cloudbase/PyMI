#include "stdafx.h"

#include "MI++.h"

using namespace MI;

void MICheckResult(MI_Result result, MI_Instance* extError = NULL)
{
    if (result != MI_RESULT_OK)
    {
        if (extError)
        {
            ::MI_Instance_Delete(extError);
        }
        // TODO: format text
        throw std::exception("MI operation failed");
    }
}

Application::Application(const std::wstring& appId)
{
    this->m_app = MI_APPLICATION_NULL;

    MI_Instance* extError = NULL;
    MICheckResult(::MI_Application_Initialize(0, appId.length() ? appId.c_str() : NULL, &extError, &this->m_app), extError);
}

Application::~Application()
{
    ::MI_Application_Close(&this->m_app);
    this->m_app = MI_APPLICATION_NULL;
}

Session::Session(Application& app, const std::wstring& protocol, const std::wstring& computerName)
{
    this->m_session = MI_SESSION_NULL;

    MI_Instance* extError = NULL;
    MICheckResult(::MI_Application_NewSession(&app.m_app, protocol.length() ? protocol.c_str() : NULL, computerName.c_str(), NULL, NULL, &extError, &this->m_session), extError);
}

Session::~Session()
{
    ::MI_Session_Close(&this->m_session, NULL, NULL);
    this->m_session = MI_SESSION_NULL;
}

unsigned Class::GetMethodCount() const
{
    MI_Uint32 count = 0;
    MICheckResult(::MI_Class_GetMethodCount(this->m_class, &count));
    return count;
}

/*
void Class::GetMethod(const wchar_t* name) const
{
    MI_ParameterSet paramSet;
    MICheckResult(::MI_Class_GetMethod(this->m_class, name, NULL, &paramSet, NULL));

    MI_Uint32 count = 0;
    MICheckResult(::MI_ParameterSet_GetParameterCount(&paramSet, &count));
    for (MI_Uint32 i = 0; i < count; i++)
    {

    }
}
*/

std::tuple<MI_Value, MI_Type, MI_Uint32> Class::operator[] (const wchar_t* name) const
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Value itemValue;
    MI_Type itemType;
    MI_Uint32 itemFlags;
    MI_Uint32 itemIndex;
    MI_Boolean valueExists;

    MICheckResult(::MI_Class_GetElement(this->m_class, name, &itemValue, &valueExists, &itemType, NULL, NULL, &itemFlags, &itemIndex));
    return std::tuple<MI_Value, MI_Type, MI_Uint32>(itemValue, itemType, itemFlags);
}

std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32> Class::operator[] (unsigned index) const
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Value itemValue;
    MI_Type itemType;
    MI_Uint32 itemFlags;
    const MI_Char* itemName;
    MI_Boolean valueExists;

    MICheckResult(::MI_Class_GetElementAt(this->m_class, index, &itemName, &itemValue, &valueExists, &itemType, NULL, NULL, &itemFlags));
    return std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32>(itemName, itemValue, itemType, itemFlags);
}

unsigned Class::GetElementsCount() const
{
    MI_Uint32 count = 0;
    MICheckResult(::MI_Class_GetElementCount(this->m_class, &count));
    return count;
}

Operation::~Operation()
{
    ::MI_Operation_Close(&this->m_operation);
    this->m_operation = MI_OPERATION_NULL;
}

void Class::Delete()
{
    if (this->m_class)
    {
        ::MI_Class_Delete(this->m_class);
        this->m_class = NULL;
    }
}

Class::~Class()
{
    Delete();
}

Operation* Session::ExecQuery(const std::wstring& ns, const std::wstring& query, const std::wstring& dialect)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_QueryInstances(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), dialect.c_str(), query.c_str(), NULL, &op);
    return new Operation(op);
}

Operation* Session::InvokeMethod(Instance& instance, const std::wstring& methodName)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_Invoke(&this->m_session, MI_OPERATIONFLAGS_BASIC_RTTI, NULL, instance.GetNamespace().c_str(), NULL, methodName.c_str(), instance.m_instance, NULL, NULL, &op);
    return new Operation(op);
}

std::tuple<MI_Value, MI_Type, MI_Uint32> Instance::operator[] (const wchar_t* name) const
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Value itemValue;
    MI_Type itemType;
    MI_Uint32 itemFlags;
    MI_Uint32 itemIndex;

    MICheckResult(::MI_Instance_GetElement(this->m_instance, name, &itemValue, &itemType, &itemFlags, &itemIndex));
    return std::tuple<MI_Value, MI_Type, MI_Uint32>(itemValue, itemType, itemFlags);
}

std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32> Instance::operator[] (unsigned index) const
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Value itemValue;
    MI_Type itemType;
    MI_Uint32 itemFlags;
    const MI_Char* itemName;

    MICheckResult(::MI_Instance_GetElementAt(this->m_instance, index, &itemName, &itemValue, &itemType, &itemFlags));
    return std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32>(itemName, itemValue, itemType, itemFlags);
}

unsigned Instance::GetElementsCount() const
{
    MI_Uint32 count = 0;
    MICheckResult(::MI_Instance_GetElementCount(this->m_instance, &count));
    return count;
}

Instance* Instance::Clone() const
{
    if (this->m_instance)
    {
        MI_Instance* newInstance = NULL;
        MICheckResult(::MI_Instance_Clone(this->m_instance, &newInstance));
        return new Instance(newInstance, true);
    }

    return NULL;
}

Class* Instance::GetClass() const
{
    MI_Class* miClass = NULL;
    MICheckResult(::MI_Instance_GetClass(this->m_instance, &miClass));
    return new Class(miClass);
}

std::wstring Instance::GetClassName()
{
    if (!this->m_className.length())
    {
        const MI_Char* className = NULL;
        MICheckResult(::MI_Instance_GetClassName(this->m_instance, &className));
        this->m_className = className;
    }
    return std::wstring(this->m_className);
}

std::wstring Instance::GetNamespace()
{
    if (!this->m_namespace.length())
    {
        const MI_Char* ns = NULL;
        MICheckResult(::MI_Instance_GetNameSpace(this->m_instance, &ns));
        this->m_namespace = ns;
    }
    return std::wstring(this->m_namespace);
}

void Instance::Delete()
{
    ::MI_Instance_Delete(this->m_instance);
    this->m_instance = NULL;
}

Instance::~Instance()
{
    if (this->m_instance && this->m_ownsInstance)
    {
        Delete();
    }
}

Instance* Operation::GetNextInstance()
{
    if (this->m_hasMoreResults)
    {
        MI_Result miResult = MI_RESULT_OK;
        const MI_Char* errMsg = NULL;
        const MI_Instance* compDetails = NULL;

        const MI_Instance *miInstance = NULL;
        MICheckResult(MI_Operation_GetInstance(&this->m_operation, &miInstance, &this->m_hasMoreResults, &miResult, &errMsg, &compDetails));
        MICheckResult(miResult);

        return new Instance((MI_Instance*)miInstance, false);
    }

    return NULL;
}
