#pragma once

#include <fstream>
#include <sstream>

#include "SerializeHeader.h"
#include "SerializeTraits.h"

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

            m_stack.push(m_doc.first_node("Object"));

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

        template <typename T>
        void operator >> (T &obj)
        {
            obj.Serialize(*this);
        }

    private:
        int GetNodeChildCount(XmlNodePtr node)
        {
            int count = 0;

            if (!node)
            {
                return 0;
            }

            for (XmlNodePtr subNode = node->first_node();
                subNode != nullptr;
                subNode = subNode->next_sibling())
            {
                count++;
            }

            return count;
        }

        XmlNodePtr GetTagNode(const char *tag)
        {
            if (m_stack.empty())
            {
                assert(false);
                return nullptr;
            }

            XmlNodePtr parent = m_stack.top();
            XmlNodePtr subNode = parent->first_node(tag);

            return subNode;
        }

        void StartObject(XmlNodePtr node)
        {
            m_stack.push(node);
        }

        void EndObject(XmlNodePtr node)
        {
            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            if (m_stack.top() != node)
            {
                assert(false);
                return;
            }


            m_stack.pop();
        }

        void StartArray(XmlNodePtr node)
        {
            StartObject(node);
        }

        void EndArray(XmlNodePtr node)
        {
            EndObject(node);
        }

    public:

#define GET_TAG_NODE_OR_RET(tag)  XmlNodePtr node = GetTagNode(tag); if (!node) return; 

        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag)

                value = (T)std::atoll(node->value());
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag)

                value = (T)std::atoll(node->value());
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag)

                value = (T)std::atoi(node->value());
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            inline   Serialize(const char *tag, T &value)
        {
            GET_TAG_NODE_OR_RET(tag)

                value = (T)std::atoi(node->value());
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag)

                value = (T)std::atof(node->value());
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag)

                value = (T)std::atoi(node->value());
        }

        void inline Serialize(const char *tag, bool &value)
        {
            GET_TAG_NODE_OR_RET(tag)

                value = (bool)std::atoi(node->value());
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            GET_TAG_NODE_OR_RET(tag)

                str = node->value();
        }

        template <typename T, int N>
        void Serialize(const char *tag, T(&array)[N])
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount && i < N; i++)
            {
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, array[i]);
            }

            EndArray(node);
        }

        template <typename T, int N1, int N2>
        void Serialize(const char *tag, T(&array)[N1][N2])
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount && i < N1; i++)
            {
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, array[i]);
            }

            EndArray(node);
        }

        template <typename T, int N1, int N2, int N3>
        void Serialize(const char *tag, T(&array)[N1][N2][N3])
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount && i < N1; i++)
            {
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, array[i]);
            }

            EndArray(node);
        }

        template <typename T>
        typename std::enable_if<std::is_class<T>::value, void>::type
            Serialize(const char *tag, T &obj)
        {
            GET_TAG_NODE_OR_RET(tag)

                StartObject(node);
            obj.Serialize(*this);
            EndObject(node);
        }

        template <typename T>
        void SerializeArrayVector(const char *tag, std::vector<T> &vec)
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount && i < (int)vec.size(); i++)
            {
                T obj;
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, obj);
                vec[i] = obj;
            }

            EndArray(node);
        }

        template <typename T>
        void Serialize(const char *tag, std::vector<T> &vec)
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount; i++)
            {
                T obj;
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, obj);
                vec.emplace_back(obj);
            }

            EndArray(node);
        }

        template <typename T>
        void Serialize(const char *tag, std::list<T> &ls)
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount; i++)
            {
                T obj;
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, obj);
                ls.emplace_back(obj);
            }

            EndArray(node);
        }

        template <typename T>
        void Serialize(const char *tag, std::stack<T> &st)
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount; i++)
            {
                T obj;
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, obj);
                st.emplace(obj);
            }

            EndArray(node);
        }

        template <typename T>
        void Serialize(const char *tag, std::deque<T> &deq)
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount; i++)
            {
                T obj;
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, obj);
                deq.emplace_back(obj);
            }

            EndArray(node);
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
        {
            GET_TAG_NODE_OR_RET(tag)

                for (XmlNodePtr subNode = node->first_node(); subNode != nullptr;
                    subNode = subNode->next_sibling())
            {
                XmlNodePtr keyNode = subNode->first_node("key");
                XmlNodePtr valueNode = subNode->first_node("value");

                if (keyNode && valueNode)
                {
                    _Kty key;
                    _Ty  value;

                    StartObject(subNode);

                    Serialize("key", key);
                    Serialize("value", value);
                    mp.insert({ key, value });

                    EndObject(subNode);
                }
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::set<T> &set)
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount; i++)
            {
                T obj;
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, obj);
                set.insert(obj);
            }

            EndArray(node);
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
        {
            GET_TAG_NODE_OR_RET(tag)

                for (XmlNodePtr subNode = node->first_node(); subNode != nullptr;
                    subNode = subNode->next_sibling())
            {
                XmlNodePtr  keyNode = subNode->first_node("key");
                XmlNodePtr  valueNode = subNode->first_node("value");

                if (keyNode && valueNode)
                {
                    _Kty key;
                    _Ty  value;

                    StartObject(subNode);

                    Serialize("key", key);
                    Serialize("value", value);
                    mp.insert({ key, value });

                    EndObject(subNode);
                }
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::unordered_set<T> &set)
        {
            GET_TAG_NODE_OR_RET(tag)

                StartArray(node);

            int childCount = GetNodeChildCount(node);
            for (int i = 0; i < childCount; i++)
            {
                T obj;
                char buffer[50] = { 0 };

                sprintf(buffer, "item%d", i);
                Serialize(buffer, obj);
                set.insert(obj);
            }

            EndArray(node);
        }

#undef GET_TAG_NODE_OR_RET

    private:
        rapidxml::xml_document<>    m_doc;
        std::stack<XmlNodePtr>      m_stack;
    };
}