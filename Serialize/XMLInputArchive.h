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

            XmlNodePtr objNode = m_doc.first_node("root");
            if (objNode)
            {
                XmlNodePtr trunkNode = objNode->first_node("object");
                if (trunkNode)
                {
                    m_stack.push(trunkNode);
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

    public:

        Node GetTagNode(const char *tag)
        {
            Node node;

            if (m_stack.empty())
            {
                return node;
            }

            XmlNodePtr parent = m_stack.top();
            if (!parent)
            {
                return node;
            }

            XmlNodePtr subNode = parent->first_node(tag);
            if (!subNode)
            {
                return node;
            }

            node.elem = subNode;

            for (XmlNodePtr childNode = subNode->first_node();
                childNode != nullptr;
                childNode = childNode->next_sibling())
            {
                node.children.push_back(Any(childNode));
            }
            
            return node;
        }

        bool HasMember(const char *tag, Any &any)
        {
            XmlNodePtr node = any.AnyCast<XmlNodePtr>();
            if (node && node->first_node(tag))
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

            m_stack.push(any.AnyCast<XmlNodePtr>());
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

            if (m_stack.top() != any.AnyCast<XmlNodePtr>())
            {
                assert(false);
                return;
            }


            m_stack.pop();
        }

    public:

#define GET_TAG_NODE_OR_RET(tag)  Node node = GetTagNode(tag); if (node.elem.IsNull()) return; 

        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoll(node.elem.AnyCast<XmlNodePtr>()->value());
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoll(node.elem.AnyCast<XmlNodePtr>()->value());
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoi(node.elem.AnyCast<XmlNodePtr>()->value());
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            inline   Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoi(node.elem.AnyCast<XmlNodePtr>()->value());
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atof(node.elem.AnyCast<XmlNodePtr>()->value());
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = (T)std::atoi(node.elem.AnyCast<XmlNodePtr>()->value());
        }

        void inline Serialize(const char *tag, bool &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = std::atoi(node.elem.AnyCast<XmlNodePtr>()->value()) == 1;
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            GET_TAG_NODE_OR_RET(tag);
            str = node.elem.AnyCast<XmlNodePtr>()->value();
        }

#undef GET_TAG_NODE_OR_RET

    private:
        rapidxml::xml_document<>    m_doc;
        std::stack<XmlNodePtr>      m_stack;
    };
}