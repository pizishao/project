#pragma once

#include "common.h"
#include "traits.h"
#include "RapidjsonWrap.h"

namespace Serialization
{
    class JsonOutPutArchive
    {
    public:
        JsonOutPutArchive() : writer(s){}
        ~JsonOutPutArchive(){}

    public:
        void StartArray(const char *tag)
        {
            writer.String(tag);
            writer.StartArray();
        }

        void EndArray(const char *tag)
        {
            (void)tag;
            writer.EndArray();
        }

        void StartObject(const char *tag)
        {
            if (tag)
            {
                writer.String(tag);
                writer.StartObject();
            }
            else
            {
                writer.StartObject();
            }
        }

        void EndObject(const char *tag)
        {
            (void)tag;
            writer.EndObject();
        }

    public:

        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer.String(tag);
            writer.Int64(value);
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer.String(tag);
            writer.Uint64(value);
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer.String(tag);
            writer.Int(value);
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer.String(tag);
            writer.UInt(value);
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            writer.String(tag);
            writer.Double(value);
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            writer.String(tag);
            writer.Int((int)value);
        }

        void inline Serialize(const char *tag, bool value)
        {
            writer.String(tag);
            writer.Bool(value);
        }

        void inline Serialize(const char *tag, std::string str)
        {
            writer.String(tag);
            writer.String(str.c_str());
        }

        std::string c_str()
        {
            return s.GetString();
        }

    private:
        rapidjson::StringBuffer                     s;
        rapidjson::Writer<rapidjson::StringBuffer>  writer;
    };
}