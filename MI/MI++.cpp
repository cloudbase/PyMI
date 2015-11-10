#include "stdafx.h"

#include "MI++.h"

using namespace MI;

Application::Application(const std::wstring& appId)
{
    this->m_app = MI_APPLICATION_NULL;

    MI_Instance* extError = NULL;
    if (::MI_Application_Initialize(0, appId.length() ? appId.c_str() : NULL, &extError, &this->m_app) != MI_RESULT_OK)
    {
        ::MI_Instance_Delete(extError);
        throw std::exception("Application");
    }
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
    if (::MI_Application_NewSession(&app.m_app, protocol.length() ? protocol.c_str(): NULL, computerName.c_str(), NULL, NULL, &extError, &this->m_session) != MI_RESULT_OK)
    {
        ::MI_Instance_Delete(extError);
        throw std::exception("Session");
    }
}

Session::~Session()
{
    ::MI_Session_Close(&this->m_session, NULL, NULL);
    this->m_session = MI_SESSION_NULL;
}

Operation::Operation()
{
    this->m_operation = MI_OPERATION_NULL;
}


Operation::~Operation()
{
    ::MI_Operation_Close(&this->m_operation);
    this->m_operation = MI_OPERATION_NULL;
}

std::tuple<MI_Value, MI_Type, MI_Uint32> Instance::operator[] (const wchar_t* name) const
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Value itemValue;
    MI_Type itemType;
    MI_Uint32 itemFlags;
    MI_Uint32 itemIndex;

    miResult = ::MI_Instance_GetElement(this->m_instance, name, &itemValue, &itemType, &itemFlags, &itemIndex);
    if (miResult != MI_RESULT_OK)
    {
        throw std::exception("operator[]");
    }

    return std::tuple<MI_Value, MI_Type, MI_Uint32>(itemValue, itemType, itemFlags);
}

std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32> Instance::operator[] (unsigned index) const
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Value itemValue;
    MI_Type itemType;
    MI_Uint32 itemFlags;
    const MI_Char* itemName;

    miResult = ::MI_Instance_GetElementAt(this->m_instance, index, &itemName, &itemValue, &itemType, &itemFlags);
    if (miResult != MI_RESULT_OK)
    {
        throw std::exception("operator[]");
    }

    return std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32>(itemName, itemValue, itemType, itemFlags);
}

unsigned Instance::GetElementsCount() const
{
    MI_Uint32 count = 0;
    if (::MI_Instance_GetElementCount(this->m_instance, &count) != MI_RESULT_OK)
    {
        throw std::exception("GetElementsCount");
    }

    return count;
}

Instance* Instance::Clone() const
{
    if (this->m_instance)
    {
        MI_Instance* newInstance = NULL;
        if (::MI_Instance_Clone(this->m_instance, &newInstance) != MI_RESULT_OK)
        {
            throw std::exception("Clone");
        }
        return new Instance(newInstance, true);
    }

    return NULL;
}

std::wstring Instance::GetClassName() const
{
    const MI_Char* className = NULL;
    if (::MI_Instance_GetClassName(this->m_instance, &className) != MI_RESULT_OK)
    {
        throw std::exception("GetClassName");
    }

    return std::wstring(className);
}

void Instance::Delete()
{
    ::MI_Instance_Delete(this->m_instance);
    this->m_instance = NULL;
}

Instance::~Instance()
{
    if (m_ownsInstance && this->m_instance)
    {
        Delete();
    }
}

Query::Query(Session& session, const std::wstring& ns, const std::wstring& query, const std::wstring& dialect)
{
    ::MI_Session_QueryInstances(&session.m_session, MI_OPERATIONFLAGS_DEFAULT_RTTI, NULL, ns.c_str(), dialect.c_str(), query.c_str(), NULL, &this->m_operation);
}

Instance Query::GetNextInstance()
{
    const MI_Instance *miInstance = NULL;
    if (this->m_hasMoreResults)
    {
        MI_Result miResult = MI_RESULT_OK;
        const MI_Char* errMsg = NULL;
        const MI_Instance* compDetails = NULL;

        MI_Result miResult2 = MI_Operation_GetInstance(&this->m_operation, &miInstance, &this->m_hasMoreResults, &miResult, &errMsg, &compDetails);
        if (miResult2 != MI_RESULT_OK || miResult != MI_RESULT_OK)
        {
            throw std::exception("GetNextInstance");
        }
    }
    return Instance((MI_Instance*)miInstance, false);
}
