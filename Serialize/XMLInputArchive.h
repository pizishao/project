#pragma once

#include <fstream>
#include <sstream>

#include "common.h"
#include "traits.h"
#include "RapidxmlWrap.h"

namespace Serialization
{
    class XmlInPutArchive
    {
    public:
        XmlInPutArchive(){}
        ~XmlInPutArchive(){}

    public:
        bool Load(const std::string xmltext)
        {
            try
            {
                m_doc.parse<0>(m_doc.allocate_string(xmltext.c_str()));
            }
            catch (rapidxml::parse_error &ex)
            {
                std::string what = ex.what();
                assert(false);
                return false;
            }

            XmlNodePtr rootNode = m_doc.first_node("root");
            if (rootNode)
            {
                XmlNodePtr objectNode = rootNode->first_node("object");
                if (objectNode)
                {
                    XmlSeriNode node;
                    node.elem = objectNode;
                    m_stack.push(node);
                }
            }

            return true;
        }

        bool LoadFromFile(std::string filename)
        {
            std::ifstream in(filename);

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
                return false;
            }

            XmlSeriNode parent = m_stack.top();
            assert(parent.elem);
            XmlNodePtr subNode = parent.elem->first_node(tag);

            if (subNode)
            {
                return true;
            }

            return false;            
        }

        XmlSeriNode GetTagNode(const char *tag)
        {
            XmlSeriNode node;

            if (m_stack.empty())
            {
                return node;
            }

            XmlSeriNode parentNode = m_stack.top();
            assert(parentNode.elem);

            XmlNodePtr subNode = parentNode.elem->first_node(tag);
            if (!subNode)
            {
                return node;
            }

            node.elem = subNode;

            for (XmlNodePtr childNode = subNode->first_node();
                childNode != nullptr;
                childNode = childNode->next_sibling())
            {
                node.children.push_back(childNode);
            }

            return node;
        }

        bool ItemHasTag(int index, const char *tag)
        {
            if (m_stack.empty())
            {
                return false;
            }

            XmlSeriNode parentNode = m_stack.top();

            if (index >= (int)parentNode.children.size())
            {
                assert(false);
                return false;
            }

            XmlNodePtr subNode = parentNode.children[index]->first_node(tag);
            if (subNode)
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

            XmlSeriNode node = GetTagNode(tag);
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

            XmlSeriNode node = GetTagNode(tag);
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

            XmlSeriNode parentNode = m_stack.top();

            if (index >= (int)parentNode.children.size())
            {
                assert(false);
                return;
            }

            XmlSeriNode subNode;
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

#define GET_TAG_NODE_OR_RET(tag)  XmlSeriNode node = GetTagNode(tag); if (!node.elem) return; 

    public:
        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoll(node.elem->value());
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoll(node.elem->value());
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoi(node.elem->value());
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            inline   Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoi(node.elem->value());
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atof(node.elem->value());
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoi(node.elem->value());
        }

        void inline Serialize(const char *tag, bool &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = std::atoi(node.elem->value()) == 1;
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            GET_TAG_NODE_OR_RET(tag);
            str = node.elem->value();
        }

#undef GET_TAG_NODE_OR_RET

    private:
        rapidxml::xml_document<>    m_doc;
        std::stack<XmlSeriNode>     m_stack;
    };
}