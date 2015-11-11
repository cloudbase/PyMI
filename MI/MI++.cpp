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

Instance* Application::NewInstance(const std::wstring& className)
{
    MI_Instance* instance = NULL;
    MICheckResult(::MI_Application_NewInstance(&this->m_app, className.c_str(), NULL, &instance));
    return new Instance(instance, true);
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

std::map<std::wstring, Qualifier> GetQualifiers(MI_QualifierSet* qualifierSet)
{
    std::map<std::wstring, Qualifier> qualifiers;

    MI_Uint32 count = 0;
    MICheckResult(::MI_QualifierSet_GetQualifierCount(qualifierSet, &count));
    for (MI_Uint32 i = 0; i < count; i++)
    {
        const MI_Char* qualifierName = NULL;
        Qualifier q;
        MICheckResult(::MI_QualifierSet_GetQualifierAt(qualifierSet, i, &qualifierName, &q.m_type, &q.m_flags, &q.m_value));
        q.m_name = qualifierName;
        qualifiers[qualifierName] = q;
    }

    return qualifiers;
}

std::map<std::wstring, ParameterInfo> GetParametersInfo(MI_ParameterSet* paramSet)
{
    std::map<std::wstring, ParameterInfo> parametersInfo;

    MI_Uint32 count = 0;
    MICheckResult(::MI_ParameterSet_GetParameterCount(paramSet, &count));
    for (MI_Uint32 i = 0; i < count; i++)
    {
        const MI_Char* paramName = NULL;
        ParameterInfo paramInfo;
        MI_QualifierSet qualifierSet;
        MICheckResult(::MI_ParameterSet_GetParameterAt(paramSet, i, &paramName, &paramInfo.m_type, NULL, &qualifierSet));
        paramInfo.m_name = paramName;
        paramInfo.m_index = i;
        paramInfo.m_qualifiers = GetQualifiers(&qualifierSet);
        parametersInfo[paramName] = paramInfo;
    }

    return parametersInfo;
}

MethodInfo Class::GetMethodInfo(const wchar_t* name) const
{
    MethodInfo info;
    MI_ParameterSet paramSet;
    MI_QualifierSet qualifierSet;
    MI_Uint32 index;
    MICheckResult(::MI_Class_GetMethod(this->m_class, name, &qualifierSet, &paramSet, &index));
    info.m_name = name;
    info.m_index = index;
    info.m_qualifiers = GetQualifiers(&qualifierSet);
    info.m_parameters = GetParametersInfo(&paramSet);
    return info;
}

MethodInfo Class::GetMethodInfo(unsigned index) const
{
    MethodInfo info;
    MI_ParameterSet paramSet;
    MI_QualifierSet qualifierSet;
    const MI_Char* name = NULL;
    MICheckResult(::MI_Class_GetMethodAt(this->m_class, index, &name, &qualifierSet, &paramSet));
    info.m_name = name;
    info.m_index = index;
    info.m_qualifiers = GetQualifiers(&qualifierSet);
    info.m_parameters = GetParametersInfo(&paramSet);
    return info;
}

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
    ::MI_Session_QueryInstances(&this->m_session, 0, NULL, ns.c_str(), dialect.c_str(), query.c_str(), NULL, &op);
    return new Operation(op);
}

Instance* Session::InvokeMethod(Instance& instance, const std::wstring& methodName, const Instance* inboundParams)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_Invoke(&this->m_session, 0, NULL, instance.GetNamespace().c_str(), NULL, methodName.c_str(), instance.m_instance, inboundParams ? inboundParams->m_instance : NULL, NULL, &op);
    Operation operation(op);
    return operation.GetNextInstance();
}

Instance* Session::InvokeMethod(const std::wstring& ns, const std::wstring& className, const std::wstring& methodName, const Instance& inboundParams)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_Invoke(&this->m_session, 0, NULL, ns.c_str(), className.c_str(), methodName.c_str(), NULL, inboundParams.m_instance, NULL, &op);
    Operation operation(op);
    return operation.GetNextInstance();
}

Class* Session::GetClass(const std::wstring& ns, const std::wstring& className)
{
    MI_Class* miClass = NULL;
    MI_Operation op;
    ::MI_Session_GetClass(&this->m_session, 0, NULL, ns.c_str(), className.c_str(), NULL, &op);
    Operation operation(op);
    return operation.GetNextClass();
}


MI_Type Instance::GetElementType(const std::wstring& name) const
{
    MI_Type itemType;
    MICheckResult(::MI_Instance_GetElement(this->m_instance, name.c_str(), NULL, &itemType, NULL, NULL));
    return itemType;
}

MI_Type Instance::GetElementType(unsigned index) const
{
    MI_Type itemType;
    MICheckResult(::MI_Instance_GetElementAt(this->m_instance, index, NULL, NULL, &itemType, NULL));
    return itemType;
}

void Instance::SetElement(const std::wstring& name, const MI_Value* value, MI_Type valueType)
{
    MICheckResult(::MI_Instance_SetElement(this->m_instance, name.c_str(), value, valueType, value ? 0 : MI_FLAG_NULL));
}

void Instance::SetElement(unsigned index, const MI_Value* value, MI_Type valueType)
{
    MICheckResult(::MI_Instance_SetElementAt(this->m_instance, index, value, valueType, value ? 0 : MI_FLAG_NULL));
}

void Instance::AddElement(const std::wstring& name, const MI_Value* value, MI_Type valueType)
{
    MICheckResult(::MI_Instance_AddElement(this->m_instance, name.c_str(), value, valueType, value ? 0 : MI_FLAG_NULL));
}

void Instance::ClearElement(const std::wstring& name)
{
    MICheckResult(::MI_Instance_ClearElement(this->m_instance, name.c_str()));
}

void Instance::ClearElement(unsigned index)
{
    MICheckResult(::MI_Instance_ClearElementAt(this->m_instance, index));
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

Class* Operation::GetNextClass()
{
    if (this->m_hasMoreResults)
    {
        MI_Result miResult = MI_RESULT_OK;
        const MI_Char* errMsg = NULL;
        const MI_Instance* compDetails = NULL;

        const MI_Class* miClass = NULL;
        MICheckResult(::MI_Operation_GetClass(&this->m_operation, &miClass, &this->m_hasMoreResults, &miResult, &errMsg, &compDetails));
        MICheckResult(miResult);

        if (miClass)
        {
            return new Class((MI_Class*)miClass);
        }
    }

    return NULL;
}

Instance* Operation::GetNextInstance()
{
    if (this->m_hasMoreResults)
    {
        MI_Result miResult = MI_RESULT_OK;
        const MI_Char* errMsg = NULL;
        const MI_Instance* compDetails = NULL;

        const MI_Instance* miInstance = NULL;
        MICheckResult(::MI_Operation_GetInstance(&this->m_operation, &miInstance, &this->m_hasMoreResults, &miResult, &errMsg, &compDetails));
        MICheckResult(miResult);

        if (miInstance)
        {
            return new Instance((MI_Instance*)miInstance, false);
        }
    }

    return NULL;
}
