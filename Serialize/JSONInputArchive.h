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
        bool Load(const std::string jsontext)
        {
            m_doc.Parse<0>(jsontext.c_str());

            if (m_doc.HasParseError())
            {
                assert(false);
                return false;
            }

            JsonNode node;
            node.elem = &m_doc;
            m_stack.push(node);

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

        bool CheckTagNodeExist(const char *tag)
        {
            if (m_stack.empty())
            {
                assert(false);
                return false;
            }

            JsonNode node = m_stack.top();

            rapidjson::Value &jVal = *node.elem;

            if (jVal.HasMember(tag))
            {
                return true;
            }

            return false;
        }

        JsonNode GetTagNode(const char *tag)
        {
            JsonNode node;

            if (m_stack.empty())
            {
                return node;
            }

            JsonNode parentNode = m_stack.top();
            rapidjson::Value &jVal = *parentNode.elem;

            if (jVal.HasMember(tag))
            {
                rapidjson::Value &tagValue = jVal[tag];
                node.elem = &tagValue;
                if (tagValue.IsArray())
                {
                    for (size_t i = 0; i < tagValue.Size(); i++)
                    {
                        node.children.push_back(&tagValue[i]);
                    }
                }
            }

            return node;
        }

        bool ItemHasTag(int index, const char *tag)
        {
            if (m_stack.empty())
            {                
                return false;
            }            

            JsonNode parentNode = m_stack.top();

            if (index >= (int)parentNode.children.size())
            {
                assert(false);
                return false;
            }

            rapidjson::Value &jVal = *parentNode.children[index];
            
            if (jVal.HasMember(tag))
            {
                return true;
            }

            return false;
        }

        void StartObject(const char *tag)
        {
            if (!CheckTagNodeExist(tag))
            {
                return;
            }

            JsonNode node = GetTagNode(tag);
            assert(node.elem);

            m_stack.push(node);
        }

        void EndObject(const char *tag)
        {
            (void)tag;

            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            m_stack.pop();
        }

        int StartArray(const char *tag)
        {
            if (!CheckTagNodeExist(tag))
            {
                return 0;
            }

            JsonNode node = GetTagNode(tag);
            assert(node.elem);

            m_stack.push(node);

            return node.children.size();
        }

        void EndArray(const char *tag)
        {
            (void)tag;

            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            m_stack.pop();
        }

        void StartItem(int index)
        {
            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            JsonNode parentNode = m_stack.top();

            if (index >= (int)parentNode.children.size())
            {
                assert(false);
                return;
            }

            JsonNode subNode;
            subNode.elem = parentNode.children[index];
            m_stack.push(subNode);
        }

        void EndItem()
        {
            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            m_stack.pop();
        }

#define GET_TAG_NODE_OR_RET(tag) JsonNode node = GetTagNode(tag); if (!node.elem) return;

    public:        
        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem->GetInt64();
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem->GetUint64();
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem->GetInt();
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            inline   Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem->GetUint();
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem->GetDouble();
        }

        void inline Serialize(const char *tag, bool &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.elem->GetBool();
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            GET_TAG_NODE_OR_RET(tag);
            str = node.elem->GetString();
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)(node.elem->GetInt());
        }             

#undef GET_TAG_NODE_OR_RET

    private:
        std::stack<JsonNode>            m_stack;
        rapidjson::Document             m_doc;
    };
}