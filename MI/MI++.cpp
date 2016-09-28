#include "stdafx.h"
#include "MI++.h"
#include "MIExceptions.h"
#include <algorithm>
#include <sstream>

using namespace MI;


static bool IsInstanceOf(const Instance& instance, const std::wstring& className)
{
    if (instance.GetClassName() == className)
    {
        return true;
    }
    else
    {
        auto cls = instance.GetClass()->GetParentClass();
        while (cls)
        {
            if (cls->GetClassName() == className)
            {
                return true;
            }
            cls = cls->GetParentClass();
        }
    }

    return false;
}

static void MICheckResult(MI_Result result, const MI_Instance* extError = nullptr)
{
    if (result != MI_RESULT_OK)
    {
        if (extError)
        {
            Instance instance((MI_Instance*)extError, false);
            if(IsInstanceOf(instance, L"MSFT_WmiError"))
            {
                MI_Char* message = instance[L"Message"]->m_value.string;
                message = message ? message : L"";

                auto errorCode = instance[L"error_code"]->m_value.uint32;

                switch(errorCode)
                {
                case WMI_ERR_TIMEOUT:
                    throw MITimeoutException(result, errorCode, message);
                default:
                    throw MIException(result, errorCode, message);
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
            auto instanceObj = std::make_shared<Instance>(instance, false);

            if (!((Callbacks*)callbackContext)->WriteError(
                std::make_shared<Operation>(*operation, false), instanceObj))
            {
                response = MI_OperationCallback_ResponseType_No;
            }

            instanceObj->SetOutOfScope();
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

static void MI_CALL MIOperationCallbackWriteMessage(MI_Operation* operation, void* callbackContext, MI_Uint32 channel,
    const MI_Char* message)
{
    if (callbackContext)
    {
        try
        {
            ((Callbacks*)callbackContext)->WriteMessage(std::make_shared<Operation>(*operation, false), channel, message);
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
            ((Callbacks*)callbackContext)->WriteProgress(
                std::make_shared<Operation>(*operation, false), activity, currentOperation, statusDescription,
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

        auto classResultObj = classResult ? std::make_shared<Class>((MI_Class*)classResult, false) : nullptr;
        auto errorDetailsObj = errorDetails ? std::make_shared<Instance>((MI_Instance*)errorDetails, false) : nullptr;

        try
        {
            ((Callbacks*)callbackContext)->ClassResult(std::make_shared<Operation>(*operation, false), classResultObj,
                moreResults ? true : false, resultCode, errorString ? errorString : L"", errorDetailsObj);

            if (classResultObj)
            {
                classResultObj->SetOutOfScope();
            }

            if (errorDetailsObj)
            {
                errorDetailsObj->SetOutOfScope();
            }
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

static void MI_CALL MIOperationCallbackInstance(MI_Operation* operation, void* callbackContext, const MI_Instance* instance,
    MI_Boolean moreResults, MI_Result resultCode, const MI_Char* errorString, const MI_Instance* errorDetails,
    MI_Result(MI_CALL* resultAcknowledgement)(MI_Operation *operation))
{
    if (callbackContext)
    {
        auto instanceObj = instance ? std::make_shared<Instance>((MI_Instance*)instance, false) : nullptr;
        auto errorDetailsObj = errorDetails ? std::make_shared<Instance>((MI_Instance*)errorDetails, false) : nullptr;

        try
        {
            ((Callbacks*)callbackContext)->InstanceResult(
                std::make_shared<Operation>(*operation, false), instanceObj, moreResults ? true : false, resultCode,
                errorString ? errorString : L"", errorDetailsObj);

            if (instanceObj)
            {
                instanceObj->SetOutOfScope();
            }
            if (errorDetailsObj)
            {
                errorDetailsObj->SetOutOfScope();
            }
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

static void MI_CALL MIOperationCallbackIndication(MI_Operation* operation, void* callbackContext, const MI_Instance* instance,
    const MI_Char* bookmark, const MI_Char* machineID, MI_Boolean moreResults, MI_Result resultCode, const MI_Char* errorString,
    const MI_Instance* errorDetails, MI_Result(MI_CALL* resultAcknowledgement)(MI_Operation *operation))
{
    if (callbackContext)
    {
        auto instanceObj = instance ? std::make_shared<Instance>((MI_Instance*)instance, false) : nullptr;
        auto errorDetailsObj = errorDetails ? std::make_shared<Instance>((MI_Instance*)errorDetails, false) : nullptr;

        try
        {
            ((Callbacks*)callbackContext)->IndicationResult(std::make_shared<Operation>(*operation, false), instanceObj,
                bookmark ? bookmark : L"", machineID ? machineID : L"", moreResults ? true : false, resultCode,
                errorString ? errorString : L"", errorDetailsObj);

            if (instanceObj)
            {
                instanceObj->SetOutOfScope();
            }
            if (errorDetailsObj)
            {
                errorDetailsObj->SetOutOfScope();
            }
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

static void MI_CALL MIOperationCallbackStreamedParameter(MI_Operation* operation, void* callbackContext, const MI_Char* parameterName, MI_Type resultType,
    const MI_Value* result, MI_Result(MI_CALL* resultAcknowledgement)(MI_Operation *operation))
{
    if (callbackContext)
    {
        try
        {
            ((Callbacks*)callbackContext)->StreamedParameterResult(std::make_shared<Operation>(*operation, false), parameterName,
                resultType, *result);
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

static MI_OperationCallbacks GetMIOperationCallbacks(std::shared_ptr<Callbacks> callback)
{
    MI_OperationCallbacks opCallbacks = MI_OPERATIONCALLBACKS_NULL;
    if (callback)
    {
        opCallbacks.callbackContext = &*callback;
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
    MI_Instance* extError = nullptr;
    MICheckResult(::MI_Application_Initialize(0, appId.length() ? appId.c_str() : nullptr, &extError, &this->m_app), extError);
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

std::shared_ptr<Instance> Application::NewInstanceFromClass(const std::wstring& className, const Class& miClass)
{
    MI_Instance* instance = nullptr;
    MICheckResult(::MI_Application_NewInstanceFromClass(&this->m_app, className.c_str(), miClass.m_class, &instance));
    return std::make_shared<Instance>(instance, true);
}

std::shared_ptr<Instance> Application::NewInstance(const std::wstring& className)
{
    MI_Instance* instance = nullptr;
    MICheckResult(::MI_Application_NewInstance(&this->m_app, className.c_str(), nullptr, &instance));
    return std::make_shared<Instance>(instance, true);
}

std::shared_ptr<Instance> Application::NewMethodParamsInstance(const Class& miClass, const std::wstring& methodName)
{
    auto methodInfo = miClass.GetMethodInfo(methodName);
    auto instance = this->NewInstance(L"__parameters");

    for (auto const &it : methodInfo->m_parameters)
    {
        auto& param = it.second;
        for (auto const &it2 : param->m_qualifiers)
        {
            std::wstring name = it2.second->m_name;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (name == L"in")
            {
                instance->AddElement(param->m_name, MIValue(param->m_type));
            }
        }
    }

    return instance;
}

std::shared_ptr<Serializer> Application::NewSerializer()
{
    MI_Serializer serializer;
    MICheckResult(::MI_Application_NewSerializer(&this->m_app, 0, L"MI_XML", &serializer));
    return std::shared_ptr<Serializer>(new Serializer(serializer));
}

std::shared_ptr<OperationOptions> Application::NewOperationOptions()
{
    MI_OperationOptions operationOptions;
    MICheckResult(::MI_Application_NewOperationOptions(&this->m_app, MI_FALSE, &operationOptions));
    return std::shared_ptr<OperationOptions>(new OperationOptions(operationOptions));
}

std::shared_ptr<DestinationOptions> Application::NewDestinationOptions()
{
    MI_DestinationOptions destinationOptions;
    MICheckResult(::MI_Application_NewDestinationOptions(&this->m_app, &destinationOptions));
    return std::shared_ptr<DestinationOptions>(new DestinationOptions(destinationOptions));
}

unsigned Class::GetMethodCount() const
{
    MI_Uint32 count = 0;
    MICheckResult(::MI_Class_GetMethodCount(this->m_class, &count));
    return count;
}

std::map<std::wstring, std::shared_ptr<Qualifier>> GetQualifiers(MI_QualifierSet* qualifierSet)
{
    std::map<std::wstring, std::shared_ptr<Qualifier>> qualifiers;

    MI_Uint32 count = 0;
    MICheckResult(::MI_QualifierSet_GetQualifierCount(qualifierSet, &count));
    for (MI_Uint32 i = 0; i < count; i++)
    {
        const MI_Char* qualifierName = nullptr;
        auto q = std::make_shared<Qualifier>();
        MICheckResult(::MI_QualifierSet_GetQualifierAt(qualifierSet, i, &qualifierName, &q->m_type, &q->m_flags, &q->m_value));
        q->m_name = qualifierName;
        qualifiers[qualifierName] = q;
    }

    return qualifiers;
}

std::map<std::wstring, std::shared_ptr<ParameterInfo>> GetParametersInfo(MI_ParameterSet* paramSet)
{
    std::map<std::wstring, std::shared_ptr<ParameterInfo>> parametersInfo;

    MI_Uint32 count = 0;
    MICheckResult(::MI_ParameterSet_GetParameterCount(paramSet, &count));
    for (MI_Uint32 i = 0; i < count; i++)
    {
        const MI_Char* paramName = nullptr;
        auto paramInfo = std::make_shared<ParameterInfo>();
        MI_QualifierSet qualifierSet;
        MICheckResult(::MI_ParameterSet_GetParameterAt(paramSet, i, &paramName, &paramInfo->m_type, nullptr, &qualifierSet));
        paramInfo->m_name = paramName;
        paramInfo->m_index = i;
        paramInfo->m_qualifiers = GetQualifiers(&qualifierSet);
        parametersInfo[paramName] = paramInfo;
    }

    return parametersInfo;
}

std::shared_ptr<MethodInfo> Class::GetMethodInfo(const std::wstring& name) const
{
    auto info = std::make_shared<MethodInfo>();
    MI_ParameterSet paramSet;
    MI_QualifierSet qualifierSet;
    MI_Uint32 index;
    MICheckResult(::MI_Class_GetMethod(this->m_class, name.c_str(), &qualifierSet, &paramSet, &index));
    info->m_name = name;
    info->m_index = index;
    info->m_qualifiers = GetQualifiers(&qualifierSet);
    info->m_parameters = GetParametersInfo(&paramSet);
    return info;
}

std::shared_ptr<MethodInfo> Class::GetMethodInfo(unsigned index) const
{
    auto info = std::make_shared<MethodInfo>();
    MI_ParameterSet paramSet;
    MI_QualifierSet qualifierSet;
    const MI_Char* name = nullptr;
    MICheckResult(::MI_Class_GetMethodAt(this->m_class, index, &name, &qualifierSet, &paramSet));
    info->m_name = name;
    info->m_index = index;
    info->m_qualifiers = GetQualifiers(&qualifierSet);
    info->m_parameters = GetParametersInfo(&paramSet);
    return info;
}

std::wstring Class::GetClassName() const
{
    const MI_Char* className = nullptr;
    MICheckResult(::MI_Class_GetClassName(this->m_class, &className));
    return className;
}

std::wstring Class::GetParentClassName() const
{
    const MI_Char* parentClassName = nullptr;
    auto result = ::MI_Class_GetParentClassName(this->m_class, &parentClassName);
    if (result != MI_RESULT_INVALID_SUPERCLASS)
    {
        MICheckResult(result);
    }
    return parentClassName ? parentClassName : L"";
}

std::wstring Class::GetNameSpace() const
{
    const MI_Char* nameSpace = nullptr;
    MICheckResult(::MI_Class_GetNameSpace(this->m_class, &nameSpace));
    return nameSpace ? nameSpace : L"";
}

std::wstring Class::GetServerName() const
{
    const MI_Char* serverName = nullptr;
    MICheckResult(::MI_Class_GetServerName(this->m_class, &serverName));
    return std::wstring(serverName ? serverName : L"");
}

std::shared_ptr<const std::vector<std::wstring>> Class::GetKey()
{
    if (!this->m_key)
    {
        auto key = std::make_shared<std::vector<std::wstring>>();
        unsigned count = this->GetElementsCount();
        for (unsigned i = 0; i < count; i++)
        {
            auto element = (*this)[i];
            if (element->m_flags & MI_FLAG_KEY)
            {
                key->push_back(element->m_name);
            }
            else
            {
                for (auto const &it : element->m_qualifiers)
                {
                    std::wstring name = it.second->m_name;
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    if (name == L"key")
                    {
                        key->push_back(element->m_name);
                    }
                }
            }
        }
        // Cache the key for subsequent calls
        this->m_key = key;
    }

    return this->m_key;
}

std::shared_ptr<ClassElement> Class::operator[] (const std::wstring& name) const
{
    auto element = std::make_shared<ClassElement>();
    MI_QualifierSet qualifierSet;
    MICheckResult(::MI_Class_GetElement(this->m_class, name.c_str(), &element->m_value, &element->m_valueExists, &element->m_type,
        nullptr, &qualifierSet, &element->m_flags, &element->m_index));
    element->m_name = name;
    element->m_qualifiers = GetQualifiers(&qualifierSet);
    return element;
}

std::shared_ptr<ClassElement> Class::operator[] (unsigned index) const
{
    auto element = std::make_shared<ClassElement>();
    MI_QualifierSet qualifierSet;
    const MI_Char* name;
    MICheckResult(::MI_Class_GetElementAt(this->m_class, index, &name, &element->m_value, &element->m_valueExists, &element->m_type,
        nullptr, &qualifierSet, &element->m_flags));
    element->m_name = name;
    element->m_index = index;
    element->m_qualifiers = GetQualifiers(&qualifierSet);
    return element;
}

unsigned Class::GetElementsCount() const
{
    MI_Uint32 count = 0;
    MICheckResult(::MI_Class_GetElementCount(this->m_class, &count));
    return count;
}

std::shared_ptr<Class> Class::GetParentClass() const
{
    MI_Class* newClass = nullptr;
    auto result = ::MI_Class_GetParentClass(this->m_class, &newClass);
    if (result != MI_RESULT_INVALID_SUPERCLASS)
    {
        MICheckResult(result);
    }
    if (newClass)
    {
        return std::make_shared<Class>(newClass, true);
    }
    else
    {
        return nullptr;
    }
}

std::shared_ptr<Class> Class::Clone() const
{
    if (this->m_class)
    {
        MI_Class* newClass = nullptr;
        MICheckResult(::MI_Class_Clone(this->m_class, &newClass));
        return std::make_shared<Class>(newClass, true);
    }
    return nullptr;
}

bool Operation::IsClosed()
{
    MI_Operation nullOp = MI_OPERATION_NULL;
    return memcmp(&this->m_operation, &nullOp, sizeof(MI_Operation)) == 0;
}

void Operation::Close()
{
    SetCurrentItem(nullptr);
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
    this->m_class = nullptr;
}

void Class::SetOutOfScope()
{
    if (!this->m_ownsInstance)
    {
        this->m_class = nullptr;
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

std::shared_ptr<Session> Application::NewSession(const std::wstring& protocol, const std::wstring& computerName,
    std::shared_ptr<DestinationOptions> destinationOptions)
{
    MI_Instance* extError = nullptr;
    MI_Session session;

    MICheckResult(::MI_Application_NewSession(&this->m_app, protocol.length() ? protocol.c_str() : nullptr, computerName.c_str(),
        destinationOptions ? &destinationOptions->m_destinationOptions : nullptr, nullptr, &extError, &session), extError);
    return std::shared_ptr<Session>(new Session(session));
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

std::shared_ptr<OperationOptions> OperationOptions::Clone() const
{
    MI_OperationOptions clonedOperationOptions;
    MICheckResult(::MI_OperationOptions_Clone(&this->m_operationOptions, &clonedOperationOptions));
    return std::shared_ptr<OperationOptions>(new OperationOptions(clonedOperationOptions));
}

void OperationOptions::Delete()
{
    ::MI_OperationOptions_Delete(&this->m_operationOptions);
    this->m_operationOptions = MI_OPERATIONOPTIONS_NULL;
}

void OperationOptions::SetCustomOption(const std::wstring& optionName,
                                       MI_Type optionValueType,
                                       const MIValue& optionValue,
                                       MI_Boolean mustComply)
{
    MICheckResult(::MI_OperationOptions_SetCustomOption(
        &this->m_operationOptions, optionName.c_str(), optionValueType,
        &optionValue.m_value, mustComply));
}

OperationOptions::~OperationOptions()
{
    MI_OperationOptions nullOperationOptions = MI_OPERATIONOPTIONS_NULL;
    if(memcmp(&this->m_operationOptions, &nullOperationOptions, sizeof(MI_OperationOptions)))
    {
        this->Delete();
    }
}

void DestinationOptions::SetUILocale(const std::wstring& locale)
{
    MICheckResult(::MI_DestinationOptions_SetUILocale(&this->m_destinationOptions, locale.c_str()));
}

std::wstring DestinationOptions::GetUILocale()
{
    const MI_Char* locale;
    MICheckResult(::MI_DestinationOptions_GetUILocale(&this->m_destinationOptions, &locale));
    return locale;
}

void DestinationOptions::SetTimeout(const MI_Interval& timeout)
{
    MICheckResult(::MI_DestinationOptions_SetTimeout(&this->m_destinationOptions, &timeout));
}

MI_Interval DestinationOptions::GetTimeout()
{
    MI_Interval timeout;
    MICheckResult(::MI_DestinationOptions_GetTimeout(&this->m_destinationOptions, &timeout));
    return timeout;
}

void DestinationOptions::SetTransport(const std::wstring& transport)
{
    MICheckResult(::MI_DestinationOptions_SetTransport(&this->m_destinationOptions, transport.c_str()));
}

std::wstring DestinationOptions::GetTransport()
{
    const MI_Char* transport;
    MICheckResult(::MI_DestinationOptions_GetTransport(&this->m_destinationOptions, &transport));
    return transport;
}

void DestinationOptions::AddCredentials(const std::wstring& authType,
                                        const std::wstring& certThumbprint) {
    MI_UserCredentials creds = { 0 };
    creds.authenticationType = authType.c_str();
    creds.credentials.certificateThumbprint = certThumbprint.c_str();

    MICheckResult(::MI_DestinationOptions_AddDestinationCredentials(&this->m_destinationOptions, &creds));
}

void DestinationOptions::AddCredentials(const std::wstring& authType, const std::wstring& domain,
                                        const std::wstring& username, const std::wstring& password) {
    MI_UserCredentials creds;
    creds.authenticationType = authType.c_str();
    creds.credentials.usernamePassword.domain = domain.c_str();
    creds.credentials.usernamePassword.username = username.c_str();
    creds.credentials.usernamePassword.password = password.c_str();

    MICheckResult(::MI_DestinationOptions_AddDestinationCredentials(&this->m_destinationOptions, &creds));
}

std::shared_ptr<DestinationOptions> DestinationOptions::Clone() const
{
    MI_DestinationOptions clonedDestinationOptions;
    MICheckResult(::MI_DestinationOptions_Clone(&this->m_destinationOptions, &clonedDestinationOptions));
    return std::shared_ptr<DestinationOptions>(new DestinationOptions(clonedDestinationOptions));
}

void DestinationOptions::Delete()
{
    ::MI_DestinationOptions_Delete(&this->m_destinationOptions);
    this->m_destinationOptions = MI_DESTINATIONOPTIONS_NULL;
}

DestinationOptions::~DestinationOptions()
{
    MI_DestinationOptions nullDestinationOptions = MI_DESTINATIONOPTIONS_NULL;
    if(memcmp(&this->m_destinationOptions, &nullDestinationOptions, sizeof(MI_DestinationOptions)))
    {
        this->Delete();
    }
}

bool Session::IsClosed()
{
    MI_Session nullSession = MI_SESSION_NULL;
    return memcmp(&this->m_session, &nullSession, sizeof(MI_Session)) == 0;
}

void Session::Close()
{
    MICheckResult(::MI_Session_Close(&this->m_session, nullptr, nullptr));
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

std::shared_ptr<Operation> Session::ExecQuery(const std::wstring& ns, const std::wstring& query, const std::wstring& dialect,
                                              std::shared_ptr<OperationOptions> operationOptions)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_QueryInstances(
        &this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI,
        operationOptions ? &operationOptions->m_operationOptions : nullptr,
        ns.c_str(), dialect.c_str(),
        query.c_str(), nullptr, &op);
    return std::make_shared<Operation>(op);
}

std::shared_ptr<Operation> Session::GetAssociators(const std::wstring& ns, const Instance& instance, const std::wstring& assocClass,
    const std::wstring& resultClass, const std::wstring& role, const std::wstring& resultRole, bool keysOnly,
    std::shared_ptr<OperationOptions> operationOptions)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_AssociatorInstances(
        &this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI,
        operationOptions ? &operationOptions->m_operationOptions : nullptr,
        ns.c_str(), instance.m_instance,
        assocClass.length() ? assocClass.c_str() : nullptr,
        resultClass.length() ? resultClass.c_str() : nullptr,
        role.length() ? role.c_str() : nullptr,
        resultRole.length() ? resultRole.c_str() : nullptr,
        keysOnly, nullptr, &op);
    return std::make_shared<Operation>(op);
}

std::shared_ptr<Operation> Session::InvokeMethod(
    Instance& instance, const std::wstring& methodName, std::shared_ptr<const Instance> inboundParams,
    std::shared_ptr<OperationOptions> operationOptions)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_Invoke(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI,
        operationOptions ? &operationOptions->m_operationOptions : nullptr,
        instance.GetNameSpace().c_str(), instance.GetClassName().c_str(), methodName.c_str(), instance.m_instance,
        inboundParams && inboundParams->GetElementsCount() > 0 ? inboundParams->m_instance : nullptr,
        nullptr, &op);
    return std::make_shared<Operation>(op);
}

std::shared_ptr<Operation> Session::InvokeMethod(
    const std::wstring& ns, const std::wstring& className, const std::wstring& methodName, std::shared_ptr<const Instance> inboundParams,
    std::shared_ptr<OperationOptions> operationOptions)
{
    MI_Operation op = MI_OPERATION_NULL;
    ::MI_Session_Invoke(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI,
        operationOptions ? &operationOptions->m_operationOptions : nullptr,
        ns.c_str(), className.c_str(), methodName.c_str(), nullptr,
        inboundParams && inboundParams->GetElementsCount() > 0 ? inboundParams->m_instance : nullptr,
        nullptr, &op);
    return std::make_shared<Operation>(op);
}

void Session::DeleteInstance(const std::wstring& ns, const Instance& instance,
                             std::shared_ptr<OperationOptions> operationOptions)
{
    MI_Operation op;
    ::MI_Session_DeleteInstance(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI,
		                        operationOptions ? &operationOptions->m_operationOptions : nullptr,
                                ns.c_str(), instance.m_instance, nullptr, &op);
    Operation operation(op);
    while (operation.HasMoreResults())
    {
        operation.GetNextInstance();
    }
}

void Session::ModifyInstance(const std::wstring& ns, const Instance& instance,
                             std::shared_ptr<OperationOptions> operationOptions)
{
    MI_Operation op;
    ::MI_Session_ModifyInstance(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI,
                                operationOptions ? &operationOptions->m_operationOptions : nullptr,
                                ns.c_str(), instance.m_instance, nullptr, &op);
    Operation operation(op);
    while (operation.HasMoreResults())
    {
        operation.GetNextInstance();
    }
}

void Session::CreateInstance(const std::wstring& ns, const Instance& instance,
                             std::shared_ptr<OperationOptions> operationOptions)
{
    MI_Operation op;
    ::MI_Session_CreateInstance(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI,
                                operationOptions ? &operationOptions->m_operationOptions : nullptr,
                                ns.c_str(), instance.m_instance, nullptr, &op);
    Operation operation(op);
    while (operation.HasMoreResults())
    {
        operation.GetNextInstance();
    }
}

std::shared_ptr<Operation> Session::GetInstance(const std::wstring& ns, const Instance& keyInstance)
{
    MI_Operation op;
    ::MI_Session_GetInstance(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, nullptr, ns.c_str(), keyInstance.m_instance, nullptr, &op);
    return std::make_shared<Operation>(op);
}

std::shared_ptr<Operation> Session::GetClass(const std::wstring& ns, const std::wstring& className)
{
    MI_Class* miClass = nullptr;
    MI_Operation op;
    ::MI_Session_GetClass(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, nullptr, ns.c_str(), className.c_str(), nullptr, &op);
    return std::make_shared<Operation>(op);
}

std::shared_ptr<Operation> Session::Subscribe(const std::wstring& ns, const std::wstring& query, std::shared_ptr<Callbacks> callbacks,
    std::shared_ptr<OperationOptions> operationOptions, const std::wstring& dialect)
{
    MI_OperationCallbacks opCallbacks = GetMIOperationCallbacks(callbacks);
    MI_Operation op;
    // TODO: Add MI_SubscriptionDeliveryOptions for WinRM case
    ::MI_Session_Subscribe(&this->m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI,
        operationOptions ? &operationOptions->m_operationOptions : nullptr,
        ns.c_str(), dialect.c_str(), query.c_str(), nullptr, callbacks ? &opCallbacks : nullptr, &op);
    return std::make_shared<Operation>(op);
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
    MICheckResult(::MI_Instance_GetElement(this->m_instance, name.c_str(), nullptr, &itemType, nullptr, nullptr));
    return itemType;
}

MI_Type Instance::GetElementType(unsigned index) const
{
    MI_Type itemType;
    MICheckResult(::MI_Instance_GetElementAt(this->m_instance, index, nullptr, nullptr, &itemType, nullptr));
    return itemType;
}

void Instance::SetElement(const std::wstring& name, const MIValue& value)
{
    MICheckResult(::MI_Instance_SetElement(this->m_instance, name.c_str(), &value.m_value, value.m_type, value.m_flags));
}

void Instance::SetElement(unsigned index, const MIValue& value)
{
    MICheckResult(::MI_Instance_SetElementAt(this->m_instance, index, &value.m_value, value.m_type, value.m_flags));
}

void Instance::AddElement(const std::wstring& name, const MIValue& value)
{
    MICheckResult(::MI_Instance_AddElement(this->m_instance, name.c_str(), &value.m_value, value.m_type, value.m_flags));
}

void Instance::ClearElement(const std::wstring& name)
{
    MICheckResult(::MI_Instance_ClearElement(this->m_instance, name.c_str()));
}

void Instance::ClearElement(unsigned index)
{
    MICheckResult(::MI_Instance_ClearElementAt(this->m_instance, index));
}

const std::vector<std::wstring>& Instance::GetKeyElementNames()
{
    if (!this->m_keyElementNames)
    {
        auto miClass = this->GetClass();
        this->m_keyElementNames = miClass->GetKey();
    }

    return *this->m_keyElementNames;
}

std::wstring Instance::GetPath()
{
    std::wstring ns = this->GetNameSpace();
    std::wstring className = this->GetClassName();
    std::wstring serverName = this->GetServerName();

    if (!serverName.length())
    {
        return L"";
    }

    std::replace(ns.begin(), ns.end(), L'/', L'\\');

    std::wostringstream o;
    o << L"\\\\" << serverName << L"\\" << ns << L":" << className << L".";

    auto key = this->GetKeyElementNames();
    if (key.empty())
    {
        throw Exception(L"Cannot get path of an instance without key elements");
    }

    bool isFirst = true;
    for (auto const &it : key)
    {
        auto element = (*this)[it.c_str()];
        if (!isFirst)
        {
            o << L",";
        }
        o << it << L"=";

        switch (element->m_type)
        {
        case MI_STRING:
            {
                std::wstring value = element->m_value.string;
                ReplaceAll(value, L"\\", L"\\\\");
                ReplaceAll(value, L"\"", L"\\\"");
                o << L"\"" << value << L"\"";
            }
            break;
        case MI_UINT8:
            o << element->m_value.uint8;
            break;
        case MI_UINT16:
            o << element->m_value.uint16;
            break;
        case MI_UINT32:
            o << element->m_value.uint32;
            break;
        case MI_UINT64:
            o << element->m_value.uint64;
            break;
        case MI_SINT8:
            o << element->m_value.sint8;
            break;
        case MI_SINT16:
            o << element->m_value.sint16;
            break;
        case MI_SINT32:
            o << element->m_value.sint32;
            break;
        case MI_SINT64:
            o << element->m_value.sint64;
            break;
        default:
            throw Exception(L"Unsupported key type in path generation");
        }

        isFirst = false;
    }
    return o.str();
}

std::shared_ptr<ValueElement> Instance::operator[] (const std::wstring& name) const
{
    auto element = std::make_shared<ValueElement>();
    MICheckResult(::MI_Instance_GetElement(this->m_instance, name.c_str(), &element->m_value, &element->m_type,
        &element->m_flags, &element->m_index));
    element->m_name = name;
    return element;
}

std::shared_ptr<ValueElement> Instance::operator[] (unsigned index) const
{
    auto element = std::make_shared<ValueElement>();
    const MI_Char* name;
    MICheckResult(::MI_Instance_GetElementAt(this->m_instance, index, &name, &element->m_value,
        &element->m_type, &element->m_flags));
    element->m_name = name;
    element->m_index = index;
    return element;
}

unsigned Instance::GetElementsCount() const
{
    MI_Uint32 count = 0;
    MICheckResult(::MI_Instance_GetElementCount(this->m_instance, &count));
    return count;
}

std::shared_ptr<Instance> Instance::Clone() const
{
    if (this->m_instance)
    {
        MI_Instance* newInstance = nullptr;
        MICheckResult(::MI_Instance_Clone(this->m_instance, &newInstance));
        return std::make_shared<Instance>(newInstance, true);
    }

    return nullptr;
}

std::shared_ptr<Class> Instance::GetClass() const
{
    MI_Class* miClass = nullptr;
    MICheckResult(::MI_Instance_GetClass(this->m_instance, &miClass));
    return std::make_shared<Class>(miClass, true);
}

std::wstring Instance::GetClassName() const
{
    const MI_Char* className = nullptr;
    MICheckResult(::MI_Instance_GetClassName(this->m_instance, &className));
    return std::wstring(className);
}

std::wstring Instance::GetNameSpace() const
{
    const MI_Char* ns = nullptr;
    MICheckResult(::MI_Instance_GetNameSpace(this->m_instance, &ns));
    return std::wstring(ns ? ns : L"");
}

std::wstring Instance::GetServerName() const
{
    const MI_Char* serverName = nullptr;
    MICheckResult(::MI_Instance_GetServerName(this->m_instance, &serverName));
    return std::wstring(serverName ? serverName : L"");
}

void Instance::Delete()
{
    MICheckResult(::MI_Instance_Delete(this->m_instance));
    this->m_instance = nullptr;
}

void Instance::SetOutOfScope()
{
    if (!this->m_ownsInstance)
    {
        this->m_instance = nullptr;
    }
    ScopedItem::SetOutOfScope();
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
        this->m_currentItem = nullptr;
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

std::shared_ptr<Class> Operation::GetNextClass()
{
    if (this->m_hasMoreResults)
    {
        MI_Result miResult = MI_RESULT_OK;
        const MI_Char* errMsg = nullptr;
        const MI_Instance* compDetails = nullptr;

        const MI_Class* miClass = nullptr;
        MICheckResult(::MI_Operation_GetClass(&this->m_operation, &miClass, &this->m_hasMoreResults, &miResult,
            &errMsg, &compDetails));
        MICheckResult(miResult, compDetails);

        if (miClass)
        {
            Class* cls = new Class((MI_Class*)miClass, false, this);
            SetCurrentItem(cls);
            return std::shared_ptr<Class>(cls);
        }
    }

    return nullptr;
}

std::shared_ptr<Instance> Operation::GetNextInstance()
{
    if (this->m_hasMoreResults)
    {
        MI_Result miResult = MI_RESULT_OK;
        const MI_Char* errMsg = nullptr;
        const MI_Instance* compDetails = nullptr;
        const MI_Instance* miInstance = nullptr;
        MICheckResult(::MI_Operation_GetInstance(&this->m_operation, &miInstance, &this->m_hasMoreResults, &miResult,
            &errMsg, &compDetails));
        MICheckResult(miResult, compDetails);

        if (miInstance)
        {
            Instance* instance = new Instance((MI_Instance*)miInstance, false, this);
            SetCurrentItem(instance);
            return std::shared_ptr<Instance>(instance);
        }
    }

    return nullptr;
}

std::shared_ptr<Instance> Operation::GetNextIndication()
{
    if (this->m_hasMoreResults)
    {
        MI_Result miResult = MI_RESULT_OK;
        const MI_Char* errMsg = nullptr;
        const MI_Instance* compDetails = nullptr;
        const MI_Instance* miInstance = nullptr;
        // TODO: Add bookmark and machineID support
        MICheckResult(::MI_Operation_GetIndication(&this->m_operation, &miInstance, nullptr, nullptr, &this->m_hasMoreResults,
            &miResult, &errMsg, &compDetails));
        MICheckResult(miResult, compDetails);

        if (miInstance)
        {
            Instance* instance = new Instance((MI_Instance*)miInstance, false, this);
            SetCurrentItem(instance);
            return std::shared_ptr<Instance>(instance);
        }
    }

    return nullptr;
}

std::wstring Serializer::SerializeInstance(const Instance& instance, bool includeClass)
{
    MI_Uint32 flags = includeClass ? MI_SERIALIZER_FLAGS_INSTANCE_WITH_CLASS : 0;
    MI_Uint32 bufferSizeNeeded = 0;
    ::MI_Serializer_SerializeInstance(&m_serializer, flags, instance.m_instance, nullptr, 0, &bufferSizeNeeded);

    MI_Uint8* buffer = new MI_Uint8[bufferSizeNeeded];
    try
    {
        MICheckResult(::MI_Serializer_SerializeInstance(&m_serializer, flags, instance.m_instance, buffer,
            bufferSizeNeeded, &bufferSizeNeeded));
        auto data = std::wstring((wchar_t*)buffer, bufferSizeNeeded / sizeof(wchar_t));
        delete[] buffer;
        return data;
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
    ::MI_Serializer_SerializeClass(&m_serializer, flags, miClass.m_class, nullptr, 0, &bufferSizeNeeded);

    MI_Uint8* buffer = new MI_Uint8[bufferSizeNeeded];
    try
    {
        MICheckResult(::MI_Serializer_SerializeClass(&m_serializer, flags, miClass.m_class, buffer, bufferSizeNeeded, &bufferSizeNeeded));
        auto data = std::wstring((wchar_t*)buffer, bufferSizeNeeded / sizeof(wchar_t));
        delete[] buffer;
        return data;
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
