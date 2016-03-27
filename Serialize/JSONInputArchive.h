#pragma once

#include <fstream>
#include <sstream>

#include "common.h"
#include "traits.h"
#include "RapidjsonWrap.h"

namespace Serialization
{
    class JsonInPutArchive
    {
    public:
        JsonInPutArchive(){}
        ~JsonInPutArchive(){}

    public:
        Node GetTagNode(const char *tag)
        {
            Node node;

            if (m_stack.empty())
            {
                assert(false);
                return node;
            }

            rapidjson::Value &jVal = *m_stack.top();

            if (jVal.HasMember(tag))
            {
                rapidjson::Value &tagValue = jVal[tag];
                node.elem = &tagValue;
                if (tagValue.IsArray())
                {
                    for (size_t i = 0; i < tagValue.Size(); i++)
                    {
                        node.children.push_back(Any(&tagValue[i]));
                    }
                }
            }
            
            return node;
        }

        bool HasMember(const char *tag, Any &any)
        {
            rapidjson::Value *val = any.AnyCast<rapidjson::Value *>();
            if (val && val->HasMember(tag))
            {
                return true;
            }

            return false;
        }

        void StartObject(Any &any)
        {
            if (any.IsNull())
            {
                return;
            }

            m_stack.push(any.AnyCast<rapidjson::Value *>());
        }

        void EndObject(Any &any)
        {
            if (any.IsNull())
            {
                return;
            }

            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            if (m_stack.top() != any.AnyCast<rapidjson::Value *>())
            {
                assert(false);
                return;
            }

            m_stack.pop();
        }

    public:

#define GET_TAG_NODE_OR_RET(tag) Node node = GetTagNode(tag); if (node.elem.IsNull()) return;

        bool Load(const std::string jsontext)
        {
            m_doc.Parse<0>(jsontext.c_str());

            if (m_doc.HasParseError())
            {
                assert(false);
                return false;
            }

            m_stack.push(&m_doc);

            return true;
        }

        bool LoadFromFile(std::string filename)
        {
            std::ifstream in(filename.c_str());

            if (!in)
            {
                return false;
            }

            std::stringstream strstream;
            strstream << in.rdbuf();

            return Load(strstream.str());
        }

        template <typename T>
        void operator >> (T &obj)
        {
            obj.Serialize(*this);
        }

        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem.AnyCast<rapidjson::Value *>()->GetInt64();
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem.AnyCast<rapidjson::Value *>()->GetUint64();
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem.AnyCast<rapidjson::Value *>()->GetInt();
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            inline   Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem.AnyCast<rapidjson::Value *>()->GetUint();
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem.AnyCast<rapidjson::Value *>()->GetDouble();
        }

        void inline Serialize(const char *tag, bool &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem.AnyCast<rapidjson::Value *>()->GetBool();
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            GET_TAG_NODE_OR_RET(tag);
            str = node.elem.AnyCast<rapidjson::Value *>()->GetString();
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)(node.elem.AnyCast<rapidjson::Value *>()->GetInt());
        }             

#undef GET_JVALUE_OR_RET

    private:
        std::stack<rapidjson::Value *>  m_stack;
        rapidjson::Document             m_doc;
    };
}