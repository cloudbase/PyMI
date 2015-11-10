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
    class Instance;

    class Session
    {
    private:
        MI_Session m_session;

    public:
        Session(Application& app, const std::wstring& protocol = L"", const std::wstring& computerName = L".");
        Operation* ExecQuery(const std::wstring& ns, const std::wstring& query, const std::wstring& dialect = L"WQL");
        Operation* Session::InvokeMethod(Instance& instance, const std::wstring& methodName);
        virtual ~Session();
    };

    class Instance;

    class MElementsEnum
    {
    public:
        virtual unsigned GetElementsCount() const = 0;
        virtual std::tuple<MI_Value, MI_Type, MI_Uint32> operator[] (const wchar_t* name) const = 0;
        virtual std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32> operator[] (unsigned index) const = 0;
    };

    class Class : public MElementsEnum
    {
    private:
        MI_Class* m_class = NULL;
        Class(MI_Class* miClass) : m_class(miClass) {}
        void Delete();

        friend Instance;

    public:
        unsigned GetElementsCount() const;
        std::tuple<MI_Value, MI_Type, MI_Uint32> operator[] (const wchar_t* name) const;
        std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32> operator[] (unsigned index) const;
        unsigned GetMethodCount() const;

        virtual ~Class();
    };

    class Instance : public MElementsEnum
    {
    private:
        MI_Instance* m_instance = NULL;
        std::wstring m_namespace;
        std::wstring m_className;
        bool m_ownsInstance = false;
        Instance(MI_Instance* instance, bool ownsInstance) : m_instance(instance), m_ownsInstance(ownsInstance) {}
        void Delete();

        friend Operation;
        friend Session;

    public:
        Instance* Instance::Clone() const;
        Class* GetClass() const;
        std::wstring GetClassName();
        std::wstring GetNamespace();
        unsigned GetElementsCount() const;
        std::tuple<MI_Value, MI_Type, MI_Uint32> operator[] (const wchar_t* name) const;
        std::tuple<const MI_Char*, MI_Value, MI_Type, MI_Uint32> operator[] (unsigned index) const;
        virtual ~Instance();
    };

    class Operation
    {
    private:
        MI_Operation m_operation;
        Operation(MI_Operation& operation) : m_operation(operation) {}
        MI_Boolean m_hasMoreResults = TRUE;

        friend Session;

    public:
        Instance* GetNextInstance();
        operator bool() { return m_hasMoreResults != FALSE; }        
        virtual ~Operation();
    };
};
