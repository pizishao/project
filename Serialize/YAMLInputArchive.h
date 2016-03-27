#pragma once

#include <fstream>
#include <sstream>

#include "common.h"
#include "YamlWrap.h"

namespace Serialization
{
    class YamlInputArchive
    {
    public:
        YamlInputArchive(){}
        ~YamlInputArchive(){}

    public:
        bool Load(const std::string yamltext)
        {
            YAML::Node root = YAML::Load(yamltext);
            m_stack.push(root);

            return true;
        }

        bool LoadFromFile(std::string filename)
        {
            YAML::Node root = YAML::LoadFile(filename);
            m_stack.push(root);

            return true;
        }

        Node GetTagNode(const char *tag)
        {
            Node node;

            if (m_stack.empty())
            {
                return node;
            }

            YamlNode parent = m_stack.top();
            YamlNode subNode = parent[tag];

            if (!subNode.IsNull())
            {
                node.elem = subNode;
                if (subNode.IsSequence())
                {
                    for (auto itr = subNode.begin(); itr != subNode.end(); itr++)
                    {
                        YamlNode childNode = *itr;
                        node.children.push_back(Any(childNode));
                    }
                }
            }

            return node;
        }

        bool HasMember(const char *tag, Any &any)
        {
            YamlNode node = any.AnyCast<YamlNode>();
            YamlNode subNode = node[tag];
            if (!subNode.IsNull())
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

            m_stack.push(any.AnyCast<YamlNode>());
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

            assert(any.AnyCast<YamlNode>() == m_stack.top());

            m_stack.pop();
        }

#define GET_TAG_NODE_OR_RET(tag) Node node = GetTagNode(tag); if (node.elem.IsNull()) return;

    public:
        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {            
            GET_TAG_NODE_OR_RET(tag);
            YamlNode yamlNode = node.elem.AnyCast<YamlNode>();
            value = yamlNode.as<T>();
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            YamlNode yamlNode = node.elem.AnyCast<YamlNode>();
            value = yamlNode.as<int>();
        }

        void inline Serialize(const char *tag, bool &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            YamlNode yamlNode = node.elem.AnyCast<YamlNode>();
            value = yamlNode.as<bool>();
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            GET_TAG_NODE_OR_RET(tag);
            YamlNode yamlNode = node.elem.AnyCast<YamlNode>();
            str = yamlNode.as<std::string>();
        }

#undef GET_TAG_NODE_OR_RET

    private:
        std::stack<YAML::Node>      m_stack;
    };
}