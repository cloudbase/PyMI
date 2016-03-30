#include "stdafx.h"
#include "MIValue.h"
#include "MI++.h"
#include "MIExceptions.h"

using namespace MI;

unsigned MIValue::GetItemSize(MI_Type valueType)
{
    switch (valueType)
    {
    case MI_BOOLEAN:
        return sizeof(MI_Boolean);
    case MI_SINT8:
        return sizeof(MI_Sint8);
    case MI_UINT8:
        return sizeof(MI_Uint8);
    case MI_SINT16:
        return sizeof(MI_Sint16);
    case MI_UINT16:
        return sizeof(MI_Uint16);
    case MI_SINT32:
        return sizeof(MI_Sint32);
    case MI_UINT32:
        return sizeof(MI_Uint32);
    case MI_SINT64:
        return sizeof(MI_Sint64);
    case MI_UINT64:
        return sizeof(MI_Uint64);
    case MI_REAL32:
        return sizeof(MI_Real32);
    case MI_REAL64:
        return sizeof(MI_Real64);
    case MI_CHAR16:
        return sizeof(MI_Char16);
    case MI_DATETIME:
        return sizeof(MI_Datetime);
    case MI_STRING:
        return sizeof(MI_Char*);
    case MI_INSTANCE:
    case MI_REFERENCE:
        return sizeof(MI_Instance*);
    default:
        throw TypeConversionException();
    }
}

void MIValue::SetNullValue()
{
    m_flags = MI_FLAG_NULL;
    ::ZeroMemory(&m_value, sizeof(m_value));
}

void MIValue::CopyString(const std::string& value)
{
    int len = (int)(value.length() + 1);
    m_value.string = new MI_Char[len];
    if (::MultiByteToWideChar(CP_ACP, 0, value.c_str(), len, m_value.string, len) != len)
    {
        delete[] m_value.string;
        throw Exception(L"MultiByteToWideChar failed");
    }
}

void MIValue::CopyWString(const std::wstring& value)
{
    auto len = value.length() + 1;
    m_value.string = new MI_Char[len];
    memcpy_s(m_value.string, len * sizeof(MI_Char), value.c_str(), len * sizeof(MI_Char));
}

void MIValue::Delete(MI_Value& value, MI_Type type)
{
    if (type & MI_ARRAY)
    {
        unsigned len = value.uint8a.size;
        for (unsigned i = 0; i < len; i++)
        {
            MI_Type itemType = MI_Type(type ^ MI_ARRAY);
            unsigned itemSize = GetItemSize(itemType);
            Delete(*((MI_Value*)&value.uint8a.data[i * itemSize]), itemType);
        }
        delete[] value.uint8a.data;
        value.uint8a.data = nullptr;
    }
    else
    {
        switch (type)
        {
        case MI_STRING:
            delete[] value.string;
            value.string = nullptr;
            break;
        }
    }
}

void MIValue::SetArrayItem(const MIValue& value, MI_Uint32 index)
{
    if (!(m_type & MI_ARRAY))
    {
        throw new Exception(L"Not an array");
    }

    if (value.m_type != (m_type ^ MI_ARRAY))
    {
        throw new Exception(L"The item's MI type must match the array's item type");
    }

    if (index >= m_value.uint8a.size)
    {
        throw new Exception(L"Array index out of range");
    }

    unsigned itemSize = this->GetItemSize(value.m_type);
    if (value.m_type == MI_STRING)
    {
        MI_Char* strDest = nullptr;
        if (value.m_value.string)
        {
            auto len = lstrlen(value.m_value.string) + 1;
            strDest = new MI_Char[len];
            memcpy_s(strDest, len * sizeof(MI_Char), value.m_value.string, len * sizeof(MI_Char));
        }
        else
        {
            throw Exception(L"Array item cannot be NULL");
        }
        memcpy_s(&m_value.uint8a.data[index * itemSize], itemSize, &strDest, itemSize);
    }
    else
    {
        memcpy_s(&m_value.uint8a.data[index * itemSize], itemSize, &value.m_value, itemSize);
    }
}

std::shared_ptr<MIValue> MIValue::CreateArray(MI_Uint32 arraySize, MI_Type type)
{
    auto self = std::make_shared<MIValue>(type);
    if (arraySize)
    {
        // All array members of the MI_Value union have "pointer", "size" members.
        // It is safe to rely on one instead of referencing value.stringa, value.booleana, etc
        unsigned itemSize = self->GetItemSize((MI_Type)(type ^ MI_ARRAY));
        self->m_value.uint8a.data = (MI_Uint8*)new MI_Uint8[itemSize * arraySize];
        ::ZeroMemory(self->m_value.uint8a.data, itemSize * arraySize);
        self->m_value.uint8a.size = (unsigned)arraySize;
    }
    self->m_flags = 0;
    return self;
}

std::shared_ptr<MIValue> MIValue::FromBoolean(MI_Boolean value)
{
    return std::make_shared<MIValue>((void*)&value, MI_BOOLEAN);
}

std::shared_ptr<MIValue> MIValue::FromSint8(MI_Sint8 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_SINT8);
}

std::shared_ptr<MIValue> MIValue::FromUint8(MI_Uint8 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_UINT8);
}

std::shared_ptr<MIValue> MIValue::FromSint16(MI_Sint16 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_SINT16);
}

std::shared_ptr<MIValue> MIValue::FromUint16(MI_Uint16 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_UINT16);
}

std::shared_ptr<MIValue> MIValue::FromChar16(MI_Char16 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_CHAR16);
}

std::shared_ptr<MIValue> MIValue::FromSint32(MI_Sint32 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_SINT32);
}

std::shared_ptr<MIValue> MIValue::FromUint32(MI_Uint32 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_UINT32);
}

std::shared_ptr<MIValue> MIValue::FromSint64(MI_Sint64 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_SINT64);
}

std::shared_ptr<MIValue> MIValue::FromUint64(MI_Uint64 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_UINT64);
}

std::shared_ptr<MIValue> MIValue::FromReal32(MI_Real32 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_REAL32);
}

std::shared_ptr<MIValue> MIValue::FromReal64(MI_Real64 value)
{
    return std::make_shared<MIValue>((void*)&value, MI_REAL64);
}

std::shared_ptr<MIValue> MIValue::FromString(const std::string& value)
{
    auto self = std::make_shared<MIValue>(MI_STRING);
    self->CopyString(value);
    self->m_flags = 0;
    return self;
}

std::shared_ptr<MIValue> MIValue::FromString(const std::wstring& value)
{
    auto self = std::make_shared<MIValue>(MI_STRING);
    self->CopyWString(value);
    self->m_flags = 0;
    return self;
}

MIValue::MIValue(MI_Type type) : m_type(type)
{
    SetNullValue();
}

std::shared_ptr<MIValue> MIValue::FromInstance(MI::Instance& value)
{
    auto self = std::make_shared<MIValue>(MI_INSTANCE);
    self->m_value.instance = value.GetMIObject();
    self->m_flags = 0;
    return self;
}

std::shared_ptr<MIValue> MIValue::FromReference(MI::Instance& value)
{
    auto self = std::make_shared<MIValue>(MI_REFERENCE);
    self->m_value.reference = value.GetMIObject();
    self->m_flags = 0;
    return self;
}

MIValue::MIValue(void* value, MI_Type type) : MIValue(type)
{
    auto size = this->GetItemSize(type);
    memcpy_s(&m_value, size, value, size);
    m_flags = 0;
}

MIValue::~MIValue()
{
    Delete(m_value, m_type);
}
