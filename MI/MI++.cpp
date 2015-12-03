#include "stdafx.h"
#include "MI++.h"
#include "MIExceptions.h"
#include <algorithm>
#include <sstream>

using namespace MI;

#define WMI_ERR_TIMEOUT 0x00040004

static void MICheckResult(MI_Result result, const MI_Instance* extError = NULL)
{
    if (result != MI_RESULT_OK)
    {
        if (extError)
        {
            Instance instance((MI_Instance*)extError, false);
            if (instance.GetClassName() == L"MSFT_WmiError")
            {
                MI_Char* message = instance[L"Message"].m_value.string;
                ValueElement errorCode = instance[L"error_code"];
                switch(errorCode.m_value.uint32)
                {
                case WMI_ERR_TIMEOUT:
                    throw MITimeoutException(result, message);
                default:
                    throw MIException(result, message);
                }
            }
        }
        throw MIException(result);
    }
}

static void ReplaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

static void MI_CALL MIOperationCallbackWriteError(MI_Operation* operation, void* callbackContext, MI_Instance* instance,
    MI_Result(MI_CALL* writeErrorResult)(_In_ MI_Operation *operation, MI_OperationCallback_ResponseType response))
{
    MI_OperationCallback_ResponseType response = MI_OperationCallback_ResponseType_Yes;

    if (callbackContext)
    {
        try
        {
            if (!((Callbacks*)callbackContext)->WriteError(Operation(*operation, false), Instance(instance, false)))
            {
                response = MI_OperationCallback_ResponseType_No;
            }
        }
        catch (std::exception&)
        {
            // Ignore
        }
    }

    if (writeErrorResult)
    {
        writeErrorResult(operation, response);
    }
}

static void MI_CALL MIOperationCallbackWriteMessage(MI_Operation* operation, void* callbackContext, MI_Uint32 channel, const MI_Char* message)
{
    if (callbackContext)
    {
        try
        {
            ((Callbacks*)callbackContext)->WriteMessage(Operation(*operation, false), channel, message);
        }
        catch (std::exception&)
        {
            // Ignore
        }
    }
}

static void MI_CALL MIOperationCallbackWriteProgress(MI_Operation* operation, void* callbackContext, const MI_Char* activity,
    const MI_Char* currentOperation, const MI_Char* statusDescription, MI_Uint32 percentageComplete, MI_Uint32 secondsRemaining)
{
    if (callbackContext)
    {
        try
        {
            ((Callbacks*)callbackContext)->WriteProgress(Operation(*operation, false), activity, currentOperation, statusDescription,
                percentageComplete, secondsRemaining);
        }
        catch (std::exception&)
        {
            // Ignore
        }
    }
}

static void MI_CALL MIOperationCallbackClass(MI_Operation* operation, void* callbackContext, const MI_Class* classResult,
    MI_Boolean moreResults, MI_Result resultCode, const MI_Char* errorString, const MI_Instance* errorDetails,
    MI_Result(MI_CALL* resultAcknowledgement)(MI_Operation* operation))
{
    if (callbackContext)
    {
        const Class* classResultObj = classResult ? new Class((MI_Class*)classResult, false) : NULL;
        const Instance* errorDetailsObj = errorDetails ? new Instance((MI_Instance*)errorDetails, false) : NULL;

        try
        {
            ((Callbacks*)callbackContext)->ClassResult(Operation(*operation, false), classResultObj, moreResults ? true : false,
                resultCode, errorString ? errorString : L"", errorDetailsObj);
        }
        catch (std::exception&)
        {
            // Ignore
        }

        if (classResultObj)
        {
            delete classResultObj;
        }

        if (errorDetailsObj)
        {
            delete errorDetailsObj;
        }
    }

    if (resultAcknowledgement)
    {
        resultAcknowledgement(operation);
    }
}

static void MI_CALL MIOperationCallbackInstance(MI_Operation* operation, void* callbackContext, const MI_Instance* instance, MI_Boolean moreResults,
    MI_Result resultCode, const MI_Char* errorString, const MI_Instance* errorDetails, MI_Result(MI_CALL* resultAcknowledgement)(MI_Operation *operation))
{
    if (callbackContext)
    {
        const Instance* instanceObj = instance ? new Instance((MI_Instance*)instance, false) : NULL;
        const Instance* errorDetailsObj = errorDetails ? new Instance((MI_Instance*)errorDetails, false) : NULL;

        try
        {
            ((Callbacks*)callbackContext)->InstanceResult(Operation(*operation, false), instanceObj, moreResults ? true : false, resultCode,
                errorString ? errorString : L"", errorDetailsObj);
        }
        catch (std::exception&)
        {
            // Ignore
        }

        if (instanceObj)
        {
            delete instanceObj;
        }

        if (errorDetailsObj)
        {
            delete errorDetailsObj;
        }
    }

    if (resultAcknowledgement)
    {
        resultAcknowledgement(operation);
    }
}

static void MI_CALL MIOperationCallbackIndication(MI_Operation* operation, void* callbackContext, const MI_Instance* instance, const MI_Char* bookmark,
    const MI_Char* machineID, MI_Boolean moreResults, MI_Result resultCode, const MI_Char* errorString, const MI_Instance* errorDetails,
    MI_Result(MI_CALL* resultAcknowledgement)(MI_Operation *operation))
{
    if (callbackContext)
    {
        const Instance* instanceObj = instance ? new Instance((MI_Instance*)instance, false) : NULL;
        const Instance* errorDetailsObj = errorDetails ? new Instance((MI_Instance*)errorDetails, false) : NULL;

        try
        {
            ((Callbacks*)callbackContext)->IndicationResult(Operation(*operation, false), instanceObj, bookmark ? bookmark : L"", machineID ? machineID : L"",
                moreResults ? true : false, resultCode, errorString ? errorString : L"", errorDetailsObj);
        }
        catch (std::exception&)
        {
            // Ignore
        }

        if (instanceObj)
        {
            delete instanceObj;
        }

        if (errorDetailsObj)
        {
            delete errorDetailsObj;
        }
    }

    if (resultAcknowledgement)
    {
        resultAcknowledgement(operation);
    }
}

static void MI_CALL MIOperationCallbackStreamedParameter(MI_Operation* operation, void* callbackContext, const MI_Char* parameterName, MI_Type resultType,
    const MI_Value* result, MI_Result(MI_CALL* resultAcknowledgement)(MI_Operation *operation))
{
    if (callbackContext)
    {
        try
        {
            ((Callbacks*)callbackContext)->StreamedParameterResult(Operation(*operation, false), parameterName, resultType, *result);
        }
        catch (std::exception&)
        {
            // Ignore
        }
    }

    if (resultAcknowledgement)
    {
        resultAcknowledgement(operation);
    }
}

static MI_OperationCallbacks GetMIOperationCallbacks(Callbacks* callback)
{
    MI_OperationCallbacks opCallbacks = MI_OPERATIONCALLBACKS_NULL;
    if (callback)
    {
        opCallbacks.callbackContext = callback;
        opCallbacks.writeError = MIOperationCallbackWriteError;
        opCallbacks.writeMessage = MIOperationCallbackWriteMessage;
        opCallbacks.writeProgress = MIOperationCallbackWriteProgress;
        opCallbacks.instanceResult = MIOperationCallbackInstance;
        opCallbacks.indicationResult = MIOperationCallbackIndication;
        opCallbacks.classResult = MIOperationCallbackClass;
        opCallbacks.streamedParameterResult = MIOperationCallbackStreamedParameter;
    }
    return opCallbacks;
}

Application::Application(const std::wstring& appId)
{
    this->m_app = MI_APPLICATION_NULL;
    MI_Instance* extError = NULL;
    MICheckResult(::MI_Application_Initialize(0, appId.length() ? appId.c_str() : NULL, &extError, &this->m_app), extError);
}

bool Application::IsClosed()
{
    MI_Application nullApp = MI_APPLICATION_NULL;
    return memcmp(&this->m_app, &nullApp, sizeof(MI_Application)) == 0;
}

Application::~Application()
{
    try
    {
        if (!this->IsClosed())
        {
            this->Close();
        }
    }
    catch (std::exception&)
    {
        // Ignore
    }
}

void Application::Close()
{
    MICheckResult(::MI_Application_Close(&this->m_app));
    this->m_app = MI_APPLICATION_NULL;
}

Instance* Application::NewInstanceFromClass(const std::wstring& className, const Class& miClass)
{
    MI_Instance* instance = NULL;
    MICheckResult(::MI_Application_NewInstanceFromClass(&this->m_app, className.c_str(), miClass.m_class, &instance));
    return new Instance(instance, true);
}

Instance* Application::NewInstance(const std::wstring& className)
{
    MI_Instance* instance = NULL;
    MICheckResult(::MI_Application_NewInstance(&this->m_app, className.c_str(), NULL, &instance));
    return new Instance(instance, true);
}

Instance* Application::NewMethodParamsInstance(const Class& miClass, const std::wstring& methodName)
{
    MI::MethodInfo methodInfo = miClass.GetMethodInfo(methodName);
    MI::Instance* instance = this->NewInstance(L"__parameters");

    try
    {
        for (auto const &it : methodInfo.m_parameters)
        {
            auto& param = it.second;
            for (auto const &it2 : param.m_qualifiers)
            {
                std::wstring name = it2.second.m_name;
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                if (name == L"in")
                {
                    instance->AddElement(param.m_name, NULL, param.m_type);
                }
            }
        }
    }
    catch (std::exception&)
    {
        delete instance;
        throw;
    }

    return instance;
}

Serializer* Application::NewSerializer()
{
    MI_Serializer serializer;
    MICheckResult(::MI_Application_NewSerializer(&this->m_app, 0, L"MI_XML", &serializer));
    return new Serializer(serializer);
}

OperationOptions* Application::NewOperationOptions()
{
    MI_OperationOptions operationOptions;
    MICheckResult(::MI_Application_NewOperationOptions(&this->m_app, MI_FALSE, &operationOptions));
    return new OperationOptions(operationOptions);
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

MethodInfo Class::GetMethodInfo(const std::wstring& name) const
{
    MethodInfo info;
    MI_ParameterSet paramSet;
    MI_QualifierSet qualifierSet;
    MI_Uint32 index;
    MICheckResult(::MI_Class_GetMethod(this->m_class, name.c_str(), &qualifierSet, &paramSet, &index));
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

std::vector<std::wstring> Class::GetKey()
{
    std::vector<std::wstring> key;
    unsigned count = this->GetElementsCount();
    for (unsigned i = 0; i < count; i++)
    {
        ClassElement element = (*this)[i];
        for (auto const &it : element.m_qualifiers)
        {
            std::wstring name = it.second.m_name;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (name == L"key")
            {
                key.push_back(element.m_name);
            }
        }
    }

    return key;
}

ClassElement Class::operator[] (const std::wstring& name) const
{
    ClassElement element;
    MI_QualifierSet qualifierSet;
    MICheckResult(::MI_Class_GetElement(this->m_class, name.c_str(), &element.m_value, &element.m_valueExists, &element.m_type, NULL, &qualifierSet, &element.m_flags, &element.m_index));
    element.m_name = name;
    element.m_qualifiers = GetQualifiers(&qualifierSet);
    return element;
}

ClassElement Class::operator[] (unsigned index) const
{
    ClassElement element;
    MI_QualifierSet qualifierSet;
    const MI_Char* name;
    MICheckResult(::MI_Class_GetElementAt(this->m_class, index, &name, &element.m_value, &element.m_valueExists, &element.m_type, NULL, &qualifierSet, &element.m_flags));
    element.m_name = name;
    element.m_index = index;
    element.m_qualifiers = GetQualifiers(&qualifierSet);
    return element;
}

unsigned Class::GetElementsCount() const
{
    MI_Uint32 count = 0;
    MICheckResult(::MI_Class_GetElementCount(this->m_class, &count));
    return count;
}

Class* Class::Clone() const
{
    if (this->m_class)
    {
        MI_Class* newClass = NULL;
        MICheckResult(::MI_Class_Clone(this->m_class, &newClass));
        return new Class(newClass, true);
    }
    return NULL;
}

bool Operation::IsClosed()
{
    MI_Operation nullOp = MI_OPERATION_NULL;
    return memcmp(&this->m_operation, &nullOp, sizeof(MI_Operation)) == 0;
}

void Operation::Close()
{
    SetCurrentItem(NULL);
    MICheckResult(::MI_Operation_Close(&this->m_operation));
    this->m_operation = MI_OPERATION_NULL;
}

Operation::~Operation()
{
    try
    {
        if (this->m_ownsInstance && !this->IsClosed())
        {
            this->Close();
        }
    }
    catch (std::exception&)
    {
        // Ignore
    }
}

void Class::Delete()
{
    MICheckResult(::MI_Class_Delete(this->m_class));
    this->m_class = NULL;
}

void Class::SetOutOfScope()
{
    if (!this->m_ownsInstance)
    {
        this->m_class = NULL;
    }
}

Class::~Class()
{
    try
    {
        if (this->m_class && this->m_ownsInstance)
        {
            Delete();
        }
    }
    catch (std::exception&)
    {
        // Ignore
    }
}

Session* Application::NewSession(const std::wstring& protocol, const std::wstring& computerName)
{
    MI_Instance* extError = NULL;
    MI_Session session;
    MICheckResult(::MI_Application_NewSession(&this->m_app, protocol.length() ? protocol.c_str() : NULL, computerName.c_str(), NULL, NULL, &extError, &session), extError);
    return new Session(session);
}

void OperationOptions::SetTimeout(const MI_Interval& timeout)
{
    MICheckResult(::MI_OperationOptions_SetTimeout(&this->m_operationOptions, &timeout));
}

MI_Interval OperationOptions::GetTimeout()
{
    MI_Interval timeout;
    MICheckResult(::MI_OperationOptions_GetTimeout(&this->m_operationOptions, &timeout));
    return timeout;
}

OperationOptions* OperationOptions::Clone()
{
    MI_OperationOptions clonedOperationOptions;
    MICheckResult(::MI_OperationOptions_Clone(&this->m_operationOptions, &clonedOperationOptions));
    return new OperationOptions(clonedOperationOptions);
}

OperationOptions::~OperationOptions()
{
    ::MI_OperationOptions_Delete(&this->m_operationOptions);
    this->m_operationOptions = MI_OPERATIONOPTIONS_NULL;
}

bool Session::IsClosed()
{
    MI_Session nullSession = MI_SESSION_NULL;
    return memcmp(&this->m_session, &nullSession, sizeof(MI_Session)) == 0;
}

void Session::Close()
{
    MICheckResult(::MI_Session_Close(&this->m_session, NULL, NULL));
    this->m_session = MI_SESSION_NULL;
}

Session::~Session()
{
    try
    {
        if (!this->IsClosed())
        {
            this->Close();
        }
    }
    catch (std::exception&)
    {
        // Ignore
    }
}

Operation* Session::ExecQuery(const std::wstring& ns, const std::wstring& query, const std::wstring& dialect)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_QueryInstances(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), dialect.c_str(), query.c_str(), NULL, &op);
    return new Operation(op);
}

Operation* Session::GetAssociators(const std::wstring& ns, const Instance& instance, const std::wstring& assocClass,
    const std::wstring& resultClass, const std::wstring& role, const std::wstring& resultRole, bool keysOnly)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_AssociatorInstances(
        &this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), instance.m_instance,
        assocClass.length() ? assocClass.c_str() : NULL,
        resultClass.length() ? resultClass.c_str() : NULL,
        role.length() ? role.c_str() : NULL,
        resultRole.length() ? resultRole.c_str() : NULL,
        keysOnly, NULL, &op);
    return new Operation(op);
}

Operation* Session::InvokeMethod(Instance& instance, const std::wstring& methodName, const Instance* inboundParams)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_Invoke(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, instance.GetNamespace().c_str(), NULL, methodName.c_str(), instance.m_instance, inboundParams ? inboundParams->m_instance : NULL, NULL, &op);
    return new Operation(op);
}

Operation* Session::InvokeMethod(const std::wstring& ns, const std::wstring& className, const std::wstring& methodName, const Instance& inboundParams)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_Invoke(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), className.c_str(), methodName.c_str(), NULL, inboundParams.m_instance, NULL, &op);
    return new Operation(op);
}

void Session::DeleteInstance(const std::wstring& ns, const Instance& instance)
{
    MI_Operation op;
    ::MI_Session_DeleteInstance(&this->m_session, 0, NULL, ns.c_str(), instance.m_instance, NULL, &op);
    Operation operation(op);
    operation.GetNextInstance();
}

void Session::ModifyInstance(const std::wstring& ns, const Instance& instance)
{
    MI_Operation op;
    ::MI_Session_ModifyInstance(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), instance.m_instance, NULL, &op);
    Operation operation(op);
    operation.GetNextInstance();
}

void Session::CreateInstance(const std::wstring& ns, const Instance& instance)
{
    MI_Operation op;
    ::MI_Session_CreateInstance(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), instance.m_instance, NULL, &op);
    Operation operation(op);
    operation.GetNextInstance();
}

Operation* Session::GetInstance(const std::wstring& ns, const Instance& keyInstance)
{
    MI_Operation op;
    ::MI_Session_GetInstance(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), keyInstance.m_instance, NULL, &op);
    return new Operation(op);
}

Operation* Session::GetClass(const std::wstring& ns, const std::wstring& className)
{
    MI_Class* miClass = NULL;
    MI_Operation op;
    ::MI_Session_GetClass(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), className.c_str(), NULL, &op);
    return new Operation(op);
}

Operation* Session::Subscribe(const std::wstring& ns, const std::wstring& query, Callbacks* callbacks,
    OperationOptions* operationOptions, const std::wstring& dialect)
{
    MI_OperationCallbacks opCallbacks = GetMIOperationCallbacks(callbacks);
    MI_Operation op;
    // TODO: Add MI_SubscriptionDeliveryOptions for WinRM case
    ::MI_Session_Subscribe(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, operationOptions ? &operationOptions->m_operationOptions : NULL,
        ns.c_str(), dialect.c_str(), query.c_str(), NULL, callbacks ? &opCallbacks : NULL, &op);
    return new Operation(op);
}

void ScopedItem::RemoveFromScopeContext()
{
    if (this->m_scopeOwner)
    {
        this->m_scopeOwner->RemoveFromScopeContext(this);
    }
}

ScopedItem::~ScopedItem()
{
    RemoveFromScopeContext();
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

std::wstring Instance::GetPath()
{
    std::wstring ns = this->GetNamespace();
    std::wstring className = this->GetClassName();
    std::wstring serverName = this->GetServerName();

    if (!serverName.length())
    {
        return L"";
    }

    std::replace(ns.begin(), ns.end(), L'/', L'\\');

    std::wostringstream o;
    o << L"\\\\" << serverName << L"\\" << ns << L":" << className << L".";

    auto key = this->GetClass()->GetKey();
    bool isFirst = true;
    for (auto const &it : key)
    {
        auto element = (*this)[it.c_str()];
        if (!isFirst)
        {
            o << L",";
        }
        // TODO: handle non strings
        if (element.m_type != MI_STRING)
        {
            throw Exception(L"Unsupported key type in path generation");
        }

        std::wstring value = element.m_value.string;
        ReplaceAll(value, L"\\", L"\\\\");
        ReplaceAll(value, L"\"", L"\\\"");

        o << it << L"=\"" << value << L"\"";
        isFirst = false;
    }
    return o.str();
}

ValueElement Instance::operator[] (const std::wstring& name) const
{
    ClassElement element;
    MICheckResult(::MI_Instance_GetElement(this->m_instance, name.c_str(), &element.m_value, &element.m_type, &element.m_flags, &element.m_index));
    element.m_name = name;
    return element;
}

ValueElement Instance::operator[] (unsigned index) const
{
    ClassElement element;
    const MI_Char* name;
    MICheckResult(::MI_Instance_GetElementAt(this->m_instance, index, &name, &element.m_value, &element.m_type, &element.m_flags));
    element.m_name = name;
    element.m_index = index;
    return element;
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
    return new Class(miClass, true);
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
        this->m_namespace = ns ? ns : L"";
    }
    return std::wstring(this->m_namespace);
}

std::wstring Instance::GetServerName()
{
    if (!this->m_serverName.length())
    {
        const MI_Char* serverName = NULL;
        MICheckResult(::MI_Instance_GetServerName(this->m_instance, &serverName));
        this->m_serverName = serverName ? serverName : L"";
    }
    return std::wstring(this->m_serverName);
}

void Instance::Delete()
{
    MICheckResult(::MI_Instance_Delete(this->m_instance));
    this->m_instance = NULL;
}

void Instance::SetOutOfScope()
{
    if (!this->m_ownsInstance)
    {
        this->m_instance = NULL;
    }
}

Instance::~Instance()
{
    try
    {
        if (this->m_instance && this->m_ownsInstance)
        {
            Delete();
        }
    }
    catch (std::exception&)
    {
        // Ignore
    }
}

void Operation::Cancel()
{
    MICheckResult(::MI_Operation_Cancel(&this->m_operation, MI_REASON_NONE));
}

void Operation::RemoveFromScopeContext(ScopedItem* item)
{
    if (item == this->m_currentItem)
    {
        this->m_currentItem = NULL;
    }
}

void Operation::SetCurrentItem(ScopedItem* currentItem)
{
    if (this->m_currentItem)
    {
        this->m_currentItem->SetOutOfScope();
    }
    this->m_currentItem = currentItem;
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
        MICheckResult(miResult, compDetails);

        if (miClass)
        {
            Class* cls = new Class((MI_Class*)miClass, false, this);
            SetCurrentItem(cls);
            return cls;
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
        MICheckResult(miResult, compDetails);

        if (miInstance)
        {
            Instance* instance = new Instance((MI_Instance*)miInstance, false, this);
            SetCurrentItem(instance);
            return instance;
        }
    }

    return NULL;
}

Instance* Operation::GetNextIndication()
{
    if (this->m_hasMoreResults)
    {
        MI_Result miResult = MI_RESULT_OK;
        const MI_Char* errMsg = NULL;
        const MI_Instance* compDetails = NULL;
        const MI_Instance* miInstance = NULL;
        // TODO: Add bookmark and machineID support
        MICheckResult(::MI_Operation_GetIndication(&this->m_operation, &miInstance, NULL, NULL, &this->m_hasMoreResults, &miResult, &errMsg, &compDetails));
        MICheckResult(miResult, compDetails);

        if (miInstance)
        {
            Instance* instance = new Instance((MI_Instance*)miInstance, false, this);
            SetCurrentItem(instance);
            return instance;
        }
    }

    return NULL;
}

std::wstring Serializer::SerializeInstance(const Instance& instance, bool includeClass)
{
    MI_Uint32 flags = includeClass ? MI_SERIALIZER_FLAGS_INSTANCE_WITH_CLASS : 0;
    MI_Uint32 bufferSizeNeeded = 0;
    ::MI_Serializer_SerializeInstance(&m_serializer, flags, instance.m_instance, NULL, 0, &bufferSizeNeeded);

    MI_Uint8* buffer = new MI_Uint8[bufferSizeNeeded];
    try
    {
        MICheckResult(::MI_Serializer_SerializeInstance(&m_serializer, flags, instance.m_instance, buffer, bufferSizeNeeded, &bufferSizeNeeded));
        return std::wstring((wchar_t*)buffer, bufferSizeNeeded / sizeof(wchar_t));
    }
    catch (std::exception&)
    {
        delete[] buffer;
        throw;
    }
}

std::wstring Serializer::SerializeClass(const Class& miClass, bool deep)
{
    MI_Uint32 flags = deep ? MI_SERIALIZER_FLAGS_CLASS_DEEP : 0;
    MI_Uint32 bufferSizeNeeded = 0;
    ::MI_Serializer_SerializeClass(&m_serializer, flags, miClass.m_class, NULL, 0, &bufferSizeNeeded);

    MI_Uint8* buffer = new MI_Uint8[bufferSizeNeeded];
    try
    {
        MICheckResult(::MI_Serializer_SerializeClass(&m_serializer, flags, miClass.m_class, buffer, bufferSizeNeeded, &bufferSizeNeeded));
        return std::wstring((wchar_t*)buffer, bufferSizeNeeded / sizeof(wchar_t));
    }
    catch (std::exception&)
    {
        delete[] buffer;
        throw;
    }

}

bool Serializer::IsClosed()
{
    MI_Serializer nullSerializer;
    ZeroMemory(&nullSerializer, sizeof(MI_Serializer));
    return memcmp(&this->m_serializer, &nullSerializer, sizeof(MI_Serializer)) == 0;
}

void Serializer::Close()
{
    MICheckResult(::MI_Serializer_Close(&m_serializer));
    ZeroMemory(&this->m_serializer, sizeof(MI_Serializer));
}

Serializer::~Serializer()
{
    try
    {
        if (!this->IsClosed())
        {
            this->Close();
        }
    }
    catch (std::exception&)
    {
        // Ignore
    }
}