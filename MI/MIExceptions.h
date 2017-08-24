#pragma once

#include <MI.h>
#include <exception>
#include <string>

#define WMI_ERR_TIMEOUT 0x00040004


namespace MI
{
    extern const wchar_t* MI_RESULT_STRINGS[27];

    class Exception : public std::exception
    {
    protected:
        std::string m_message;
        void SetMessageFromWString(const std::wstring& message);

    public:
        Exception(const std::wstring& message) { SetMessageFromWString(message); };
        const char* what() const noexcept;
    };

    class MIException : public Exception
    {
    private:
        const MI_Result m_result;
        const MI_Uint32 m_errorCode;
        std::wstring MIResultToWString(MI_Result miResult) const;

    public:
        MIException(MI_Result result, MI_Uint32 errorCode = 0, const std::wstring& message = L"");
        MI_Result GetResult() const { return m_result; }
        MI_Uint32 GetErrorCode() const { return m_errorCode; }
    };

    class MITimeoutException : public MIException
    {
    public:
        MITimeoutException(MI_Result result = MI_RESULT_FAILED,
                           MI_Uint32 errorCode = WMI_ERR_TIMEOUT,
                           const std::wstring& message = L"A timeout occurred") : MIException(result, errorCode, message) {}
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
