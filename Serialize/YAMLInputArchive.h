#pragma once

#include <fstream>
#include <sstream>

#include "common.h"
#include "YamlHeader.h"

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
            YamlNode root = YAML::Load(yamltext);

            YamlSeriNode node;
            node.elem = root;

            m_stack.push(node);

            return true;
        }

        bool LoadFromFile(std::string filename)
        {
            YamlNode root = YAML::LoadFile(filename);
            
            YamlSeriNode node;
            node.elem = root;

            m_stack.push(node);

            return true;
        }

        bool CheckTagNodeExist(const char *tag)
        {
            if (m_stack.empty())
            {
                return false;
            }

            YamlSeriNode parent = m_stack.top();
            assert(!parent.elem.IsNull());
            YamlNode subNode = parent.elem[tag];

            if (!subNode.IsNull())
            {
                return true;
            }

            return false;
        }

        YamlSeriNode GetTagNode(const char *tag)
        {
            YamlSeriNode node;

            if (m_stack.empty())
            {
                return node;
            }

            YamlSeriNode parent = m_stack.top();
            YamlNode    subNode = parent.elem[tag];

            if (!subNode.IsNull())
            {
                node.elem = subNode;
                if (subNode.IsSequence())
                {
                    for (auto itr = subNode.begin(); itr != subNode.end(); itr++)
                    {
                        YamlNode childNode = *itr;
                        node.children.push_back(childNode);
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

            YamlSeriNode parentNode = m_stack.top();

            if (index >= (int)parentNode.children.size())
            {
                assert(false);
                return false;
            }

            YamlNode subNode = parentNode.children[index][tag];
            if (!subNode.IsNull())
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

            YamlSeriNode node = GetTagNode(tag);
            assert(!node.elem.IsNull());

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

            YamlSeriNode node = GetTagNode(tag);
            assert(!node.elem.IsNull());

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

            YamlSeriNode parentNode = m_stack.top();

            if (index >= (int)parentNode.children.size())
            {
                assert(false);
                return;
            }

            YamlSeriNode subNode;
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

#define GET_TAG_NODE_OR_RET(tag) YamlSeriNode node = GetTagNode(tag); if (node.elem.IsNull()) return;

    public:
        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {            
            GET_TAG_NODE_OR_RET(tag);
            YamlNode yamlNode = node.elem;
            value = yamlNode.as<T>();
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            YamlNode yamlNode = node.elem;
            value = yamlNode.as<int>();
        }

        void inline Serialize(const char *tag, bool &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            YamlNode yamlNode = node.elem;
            value = yamlNode.as<bool>();
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            GET_TAG_NODE_OR_RET(tag);
            YamlNode yamlNode = node.elem;
            str = yamlNode.as<std::string>();
        }

#undef GET_TAG_NODE_OR_RET

    private:
        std::stack<YamlSeriNode>        m_stack;
    };
}