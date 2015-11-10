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
        MI_Instance* m_instance = NULL;
        bool m_ownsInstance = false;
        Instance(MI_Instance* instance, bool ownsInstance) : m_instance(instance), m_ownsInstance(ownsInstance) {}
        void Delete();

        friend Query;

    public:
        operator bool() const { return m_instance != NULL; }
        Instance* Instance::Clone() const;
        std::wstring GetClassName() const;
        unsigned GetElementsCount() const;
        std::tuple<MI_Value, MI_Type, MI_Uint32> operator[] (const wchar_t* name) const;
        std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32> operator[] (unsigned index) const;
        virtual ~Instance();
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
