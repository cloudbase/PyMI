#pragma once

#include <MI.h>
#include <exception>
#include <string>

namespace MI
{
    class Exception : public std::exception
    {
    protected:
        std::string m_message;
        void SetMessageFromWString(const std::wstring& message);

    public:
        Exception(const std::wstring& message) { SetMessageFromWString(message); };
        const char* what() const;
    };

    class MIException : public Exception
    {
    private:
        const MI_Result m_result;
        std::wstring MIResultToWString(MI_Result miResult) const;

    public:
        MIException(MI_Result result, const std::wstring& message = L"");
    };

    class MITimeoutException : public MIException
    {
    public:
        MITimeoutException(MI_Result result = MI_RESULT_FAILED, const std::wstring& message = L"A timeout occurred") : MIException(result, message) {}
    };

    class OutOfMemoryException : public Exception
    {
    public:
        OutOfMemoryException() : Exception(L"Out of memory") {}
    };

    class TypeConversionException : public Exception
    {
    public:
        TypeConversionException(const std::wstring& message = L"Unsupported type conversion") : Exception(message) {}
    };
};
