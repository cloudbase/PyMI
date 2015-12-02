#pragma once

#include <MI.h>
#include <string>
#include <tuple>
#include <map>
#include <vector>

namespace MI
{
    class Session;
    class Instance;
    class Operation;
    class Class;
    class Serializer;
    class OperationOptions;

    class Callbacks
    {
    public:
        virtual bool WriteError(Operation& operation, const Instance& instance)
        {
            return true;
        }
        virtual void WriteMessage(Operation& operation, unsigned channel, const std::wstring& message)
        {
        }
        virtual void WriteProgress(Operation& operation, const std::wstring& activity, const std::wstring& currentOperation,
            const std::wstring& statusDescription, unsigned percentageComplete,
            unsigned secondsRemaining)
        {
        }
        virtual void ClassResult(Operation& operation, const Class* miClass, bool moreResults, MI_Result resultCode,
            const std::wstring& errorString, const Instance* errorDetails)
        {
        }
        virtual void InstanceResult(Operation& operation, const Instance* instance, bool moreResults, MI_Result resultCode,
            const std::wstring& errorString, const Instance* errorDetails)
        {
        }
        virtual void IndicationResult(Operation& operation, const Instance* instance, const std::wstring& bookmark,
            const std::wstring& machineID, bool moreResults, MI_Result resultCode,
            const std::wstring& errorString, const Instance* errorDetails)
        {
        }
        virtual void StreamedParameterResult(Operation& operation, const std::wstring& parameterName, MI_Type resultType, const MI_Value& result)
        {
        }
        virtual ~Callbacks()
        {
        }
    };

    class Application
    {
    private:
        MI_Application m_app;
        Application(const Application &obj) {}

        friend class Session;

    public:
        Application(const std::wstring& appId = L"");
        void Close();
        bool IsClosed();
        virtual ~Application();
        Instance* NewInstance(const std::wstring& className);
        Instance* NewMethodParamsInstance(const Class& miClass, const std::wstring& methodName);
        Instance* NewInstanceFromClass(const std::wstring& className, const Class& miClass);
        Session* NewSession(const std::wstring& protocol = L"", const std::wstring& computerName = L".");
        OperationOptions* NewOperationOptions();
        Serializer* NewSerializer();
    };

    class OperationOptions
    {
    private:
        MI_OperationOptions m_operationOptions;
        OperationOptions(MI_OperationOptions operationOptions) : m_operationOptions(operationOptions) {}

        friend Application;
        friend Session;

    public:
        OperationOptions* Clone();
        void SetTimeout(const MI_Interval& timeout);
        MI_Interval GetTimeout();
        virtual ~OperationOptions();
    };

    class Session
    {
    private:
        MI_Session m_session;
        Session(MI_Session session) : m_session(session) {}
        Session(const Session &obj) {}

        friend Application;

    public:
        Operation* ExecQuery(const std::wstring& ns, const std::wstring& query, const std::wstring& dialect = L"WQL");
        Operation* InvokeMethod(Instance& instance, const std::wstring& methodName, const Instance* inboundParams);
        Operation* InvokeMethod(const std::wstring& ns, const std::wstring& className, const std::wstring& methodName, const Instance& inboundParams);
        void CreateInstance(const std::wstring& ns, const Instance& instance);
        void ModifyInstance(const std::wstring& ns, const Instance& instance);
        void DeleteInstance(const std::wstring& ns, const Instance& instance);
        Operation* GetClass(const std::wstring& ns, const std::wstring& className);
        Operation* GetInstance(const std::wstring& ns, const Instance& keyInstance);
        Operation* GetAssociators(const std::wstring& ns, const Instance& instance, const std::wstring& assocClass = L"",
            const std::wstring& resultClass = L"", const std::wstring& role = L"",
            const std::wstring& resultRole = L"", bool keysOnly = false);
        Operation* Subscribe(const std::wstring& ns, const std::wstring& query, Callbacks* callback=NULL,
            OperationOptions* operationOptions=NULL, const std::wstring& dialect = L"WQL");
        void Close();
        bool IsClosed();
        virtual ~Session();
    };

    struct Qualifier
    {
    public:
        std::wstring m_name;
        MI_Type m_type;
        MI_Value m_value;
        MI_Uint32 m_flags;
    };

    struct BaseElementInfo
    {
    public:
        std::wstring m_name;
        unsigned m_index;
        MI_Type m_type;
    };

    struct ParameterInfo : public BaseElementInfo
    {
    public:
        std::map<std::wstring, Qualifier> m_qualifiers;
    };

    struct MethodInfo
    {
    public:
        std::wstring m_name;
        unsigned m_index;
        std::map<std::wstring, Qualifier> m_qualifiers;
        std::map<std::wstring, ParameterInfo> m_parameters;
    };

    struct BaseElementInfoWithFlags : public BaseElementInfo
    {
    public:
        MI_Uint32 m_flags;
    };

    struct ValueElement : public BaseElementInfoWithFlags
    {
    public:
        MI_Value m_value;
    };

    struct ClassElement : public ValueElement
    {
    public:
        MI_Boolean m_valueExists;
        std::map<std::wstring, Qualifier> m_qualifiers;
    };

    class Class
    {
    private:
        MI_Class* m_class = NULL;
        bool m_ownsInstance = false;

        Class(const Class &obj) {} // Use Clone
        void Delete();

        friend Application;
        friend Instance;
        friend Operation;
        friend Serializer;

    public:
        Class(MI_Class* miClass, bool ownsInstance) : m_class(miClass), m_ownsInstance(ownsInstance) {}
        unsigned GetElementsCount() const;
        std::vector<std::wstring> GetKey();
        ClassElement operator[] (const std::wstring& name) const;
        ClassElement operator[] (unsigned index) const;
        unsigned GetMethodCount() const;
        MethodInfo GetMethodInfo(const std::wstring& name) const;
        MethodInfo GetMethodInfo(unsigned index) const;
        Class* Clone() const;
        virtual ~Class();
    };

    class Instance
    {
    private:
        MI_Instance* m_instance = NULL;
        std::wstring m_namespace;
        std::wstring m_className;
        std::wstring m_serverName;
        bool m_ownsInstance = false;

        Instance(const Instance &obj) {} // Use Clone
        void Delete();

        friend Application;
        friend Operation;
        friend Session;
        friend Serializer;

    public:
        Instance(MI_Instance* instance, bool ownsInstance) : m_instance(instance), m_ownsInstance(ownsInstance) {}
        MI_Instance* GetMIObject() { return this->m_instance; }
        Instance* Instance::Clone() const;
        Class* GetClass() const;
        std::wstring GetClassName();
        std::wstring GetNamespace();
        std::wstring GetServerName();
        unsigned GetElementsCount() const;
        std::wstring GetPath();
        ValueElement operator[] (const std::wstring& name) const;
        ValueElement operator[] (unsigned index) const;
        void AddElement(const std::wstring& name, const MI_Value* value, MI_Type valueType);
        void SetElement(const std::wstring& name, const MI_Value* value, MI_Type valueType);
        void SetElement(unsigned index, const MI_Value* value, MI_Type valueType);
        MI_Type GetElementType(const std::wstring& name) const;
        MI_Type GetElementType(unsigned index) const;
        void ClearElement(const std::wstring& name);
        void ClearElement(unsigned index);
        virtual ~Instance();
    };

    class Operation
    {
    private:
        MI_Operation m_operation;
        MI_Boolean m_hasMoreResults = TRUE;
        bool m_ownsInstance = false;

        Operation(const Operation &obj) {}

        friend Session;

    public:
        Operation(MI_Operation& operation, bool ownsInstance=true) : m_operation(operation), m_ownsInstance(ownsInstance) {}
        Instance* GetNextInstance();
        Class* GetNextClass();
        Instance* GetNextIndication();
        operator bool() { return m_hasMoreResults != FALSE; }
        void Cancel();
        void Close();
        bool IsClosed();
        virtual ~Operation();
    };

    class Serializer
    {
    private:
        MI_Serializer m_serializer;
        Serializer(MI_Serializer& serializer) : m_serializer(serializer) {}
        Serializer(const Serializer &obj) {} // Use Clone

        friend Application;

    public:
        std::wstring SerializeInstance(const Instance& instance, bool includeClass=false);
        std::wstring SerializeClass(const Class& miClass, bool deep=false);
        void Close();
        bool IsClosed();
        virtual ~Serializer();
    };
};
