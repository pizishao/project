#pragma once

#include "common.h"
#include "traits.h"
#include "YamlHeader.h"

namespace Serialization
{ 
    class YamlOutputArchive
    {
    public:
        YamlOutputArchive(){}
        ~YamlOutputArchive(){}

    public:
        void StartArray(const char *tag)
        {
            writer << YAML::Key << tag;
            writer << YAML::Value << YAML::BeginSeq;
        }

        void EndArray(const char *tag)
        {
            (void)tag;
            writer << YAML::EndSeq;
        }

        void StartObject(const char *tag)
        {
            if (tag)
            {
                writer << YAML::Key << tag;
                writer << YAML::Value << YAML::BeginMap;
            }
            else
            {
                writer << YAML::BeginMap;
            }
        }

        void EndObject(const char *tag)
        {
            (void)tag;
            writer << YAML::EndMap;
        }

    public:
        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer << YAML::Key << tag;
            writer << YAML::Value << value;
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            writer << YAML::Key << tag;
            writer << YAML::Value << (int)value;
        }

        void inline Serialize(const char *tag, bool value)
        {
            writer << YAML::Key << tag;
            writer << YAML::Value << value;
        }

        void inline Serialize(const char *tag, std::string str)
        {
            writer << YAML::Key << tag;
            writer << YAML::Value << str;
        }       

        std::string c_str()
        {
            return writer.c_str();
        }

    private:
        YAML::Emitter   writer;
    };
}