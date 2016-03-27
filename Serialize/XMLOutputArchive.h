#pragma once

#include "common.h"
#include "traits.h"
#include "RapidxmlWrap.h"

namespace Serialization
{
    class XmlOutPutArchive
    {
    public:
        XmlOutPutArchive()
        {
            XmlNodePtr declaration = m_doc.allocate_node(rapidxml::node_declaration);;
            XmlAttributePtr version = m_doc.allocate_attribute("version", "1.0");
            declaration->append_attribute(version);

            m_doc.append_node(declaration);

            XmlNodePtr root = m_doc.allocate_node(rapidxml::node_element,
                m_doc.allocate_string("root"), nullptr);
            m_doc.append_node(root);

            m_stack.push(root);
        }

        ~XmlOutPutArchive(){}

    private:
        void WriteValue(const char *tag, std::string value)
        {
            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            XmlNodePtr parent = m_stack.top();
            parent->append_node(m_doc.allocate_node(rapidxml::node_element, m_doc.allocate_string(tag),
                m_doc.allocate_string(value.c_str())));
        }

    public:
        void StartObject(const char *tag)
        {
            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            XmlNodePtr parent = m_stack.top();
            XmlNodePtr node;
            
            if (tag)
            {
                node = m_doc.allocate_node(rapidxml::node_element, m_doc.allocate_string(tag), nullptr);
            }            
            else
            {
                node = m_doc.allocate_node(rapidxml::node_element, m_doc.allocate_string("object"), nullptr);
            }

            parent->append_node(node);
            m_stack.push(node);
        }

        void EndObject(const char *tag)
        {
            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            XmlNodePtr node = m_stack.top();

            if (tag)
            {
                if (strcmp(node->name(), tag) != 0)
                {
                    assert(false);
                    return;
                }
            }
            else
            {
                if (strcmp(node->name(), "object") != 0)
                {
                    assert(false);
                    return;
                }
            }

            m_stack.pop();
        }

        void StartArray(const char *tag)
        {
            StartObject(tag);   // direct call 
        }

        void EndArray(const char *tag)
        {
            EndObject(tag);     // direct call 
        }

    public:
        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            WriteValue(tag, std::to_string(value));
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            WriteValue(tag, std::to_string((int)value));
        }

        void inline Serialize(const char *tag, bool value)
        {
            WriteValue(tag, std::to_string((int)value));
        }

        void inline Serialize(const char *tag, std::string str)
        {
            WriteValue(tag, str);
        }

        std::string c_str()
        {
            std::string text;

            assert(!m_stack.empty() && m_stack.top() == m_doc.first_node("root"));
            rapidxml::print(std::back_inserter(text), m_doc, 0);

            return text;
        }

    private:
        rapidxml::xml_document<>    m_doc;
        std::stack<XmlNodePtr>      m_stack;
    };
}