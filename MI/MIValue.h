#pragma once
#include <MI.h>
#include <memory>

namespace MI
{
    class Instance;
    class OperationOptions;

    class MIValue
    {
    private:
        MI_Value m_value;
        MI_Type m_type;
        MI_Uint32 m_flags = 0;

        void Delete(MI_Value& value, MI_Type type);
        void SetNullValue();
        void CopyString(const std::string& value);
        void CopyWString(const std::wstring& value);

        friend Instance;
        friend OperationOptions;

    public:
        static std::shared_ptr<MIValue> FromBoolean(MI_Boolean value);
        static std::shared_ptr<MIValue> FromSint8(MI_Sint8 value);
        static std::shared_ptr<MIValue> FromUint8(MI_Uint8 value);
        static std::shared_ptr<MIValue> FromSint16(MI_Sint16 value);
        static std::shared_ptr<MIValue> FromUint16(MI_Uint16 value);
        static std::shared_ptr<MIValue> FromChar16(MI_Char16 value);
        static std::shared_ptr<MIValue> FromSint32(MI_Sint32 value);
        static std::shared_ptr<MIValue> FromUint32(MI_Uint32 value);
        static std::shared_ptr<MIValue> FromSint64(MI_Sint64 value);
        static std::shared_ptr<MIValue> FromUint64(MI_Uint64 value);
        static std::shared_ptr<MIValue> FromReal32(MI_Real32 value);
        static std::shared_ptr<MIValue> FromReal64(MI_Real64 value);
        static std::shared_ptr<MIValue> FromString(const std::string& value);
        static std::shared_ptr<MIValue> FromString(const std::wstring& value);
        static std::shared_ptr<MIValue> FromInstance(MI::Instance& value);
        static std::shared_ptr<MIValue> FromReference(MI::Instance& value);
        static std::shared_ptr<MIValue> CreateArray(MI_Uint32 arraySize, MI_Type type);
        void SetArrayItem(const MIValue& value, MI_Uint32 index);

        MIValue(void* value, MI_Type type);
        MIValue(MI_Type type);
        MIValue(const MI_Value& value, MI_Type type, MI_Uint32 flags = 0) : m_value(value), m_type(type), m_flags(flags) {}
        virtual ~MIValue();

        static unsigned GetItemSize(MI_Type valueType);
    };
}
