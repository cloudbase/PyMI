#include "stdafx.h"
#include "MIExceptions.h"
#include <locale>
#include <codecvt>
#include <cstdint>


using namespace MI;

const wchar_t* MI::MI_RESULT_STRINGS[] =
{
    L"MI_RESULT_OK",
    L"MI_RESULT_FAILED",
    L"MI_RESULT_ACCESS_DENIED",
    L"MI_RESULT_INVALID_NAMESPACE",
    L"MI_RESULT_INVALID_PARAMETER",
    L"MI_RESULT_INVALID_CLASS",
    L"MI_RESULT_NOT_FOUND",
    L"MI_RESULT_NOT_SUPPORTED",
    L"MI_RESULT_CLASS_HAS_CHILDREN",
    L"MI_RESULT_CLASS_HAS_INSTANCES",
    L"MI_RESULT_INVALID_SUPERCLASS",
    L"MI_RESULT_ALREADY_EXISTS",
    L"MI_RESULT_NO_SUCH_PROPERTY",
    L"MI_RESULT_TYPE_MISMATCH",
    L"MI_RESULT_QUERY_LANGUAGE_NOT_SUPPORTED",
    L"MI_RESULT_INVALID_QUERY",
    L"MI_RESULT_METHOD_NOT_AVAILABLE",
    L"MI_RESULT_METHOD_NOT_FOUND",
    L"MI_RESULT_NAMESPACE_NOT_EMPTY",
    L"MI_RESULT_INVALID_ENUMERATION_CONTEXT",
    L"MI_RESULT_INVALID_OPERATION_TIMEOUT",
    L"MI_RESULT_PULL_HAS_BEEN_ABANDONED",
    L"MI_RESULT_PULL_CANNOT_BE_ABANDONED",
    L"MI_RESULT_FILTERED_ENUMERATION_NOT_SUPPORTED",
    L"MI_RESULT_CONTINUATION_ON_ERROR_NOT_SUPPORTED",
    L"MI_RESULT_SERVER_LIMITS_EXCEEDED",
    L"MI_RESULT_SERVER_IS_SHUTTING_DOWN"
};

const char* Exception::what() const noexcept
{
    return this->m_message.c_str();
}

void Exception::SetMessageFromWString(const std::wstring& message)
{
    // Convert to UTF8
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
    this->m_message = cv.to_bytes(message).c_str();
}

std::wstring MIException::MIResultToWString(MI_Result miResult) const
{

    if (miResult < sizeof(MI_RESULT_STRINGS) / sizeof(MI_RESULT_STRINGS[0]))
    {
        return MI_RESULT_STRINGS[miResult];
    }
    else
    {
        return L"Unknown MI_Result";
    }
}

MIException::MIException(MI_Result result, MI_Uint32 errorCode, const std::wstring& message) :
    m_result(result), m_errorCode(errorCode), Exception(message)
{
    if (!message.length())
    {
        this->SetMessageFromWString(this->MIResultToWString(this->m_result));
    }
}
