#pragma once

#include <MI.h>
#include <string>
#include <tuple>

namespace MI
{
    class Session;

    class Application
    {
    private:
        MI_Application m_app;
        friend class Session;

    public:
        Application(const std::wstring& appId = L"");
        virtual ~Application();
    };

    class Operation;
    class Query;

    class Session
    {
    private:
        MI_Session m_session;
        friend Operation;
        friend Query;

    public:
        Session(Application& app, const std::wstring& protocol = L"", const std::wstring& computerName = L".");
        virtual ~Session();
    };

    class Operation
    {
    protected:
        MI_Operation m_operation;
        Operation();

    public:        
        virtual ~Operation();
    };

    class Query;

    class Instance
    {
    private:
        const MI_Instance *m_instance = NULL;
        Instance(const MI_Instance *instance) : m_instance(instance) {}

        friend Query;

    public:
        operator bool() { return m_instance != NULL; }
        std::tuple<MI_Value, MI_Type> operator[] (const wchar_t* name);
    };

    class Query : public virtual Operation
    {
    private:
        MI_Boolean m_hasMoreResults = TRUE;

    public:
        Query(Session& session, const std::wstring& ns, const std::wstring& query, const std::wstring& dialect = L"WQL");
        Instance Query::GetNextInstance();
        operator bool() { return m_hasMoreResults != FALSE; }        
    };
};
