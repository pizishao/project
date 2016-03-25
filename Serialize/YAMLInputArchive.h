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
        void Load(const std::string yamltext)
        {
            YAML::Node root = YAML::Load(yamltext);
            m_stack.push(root);
        }

        void LoadFromFile(std::string filename)
        {
            YAML::Node root = YAML::LoadFile(filename);
            m_stack.push(root);
        }

        template <typename T>
        void operator >> (T &obj)
        {
            obj.Serialize(*this);
        }

    private:
        YamlNode NullNode()
        {
            static YamlNode nullNode = YamlNode();
            return nullNode;
        }

        YamlNode GetTagNode(const char *tag)
        {
            if (m_stack.empty())
            {
                return NullNode();
            }

            YamlNode parent = m_stack.top();

            return parent[tag];
        }

        void StartObject(YamlNode node)
        {
            m_stack.push(node);
        }

        void EndObject(YamlNode node)
        {
            if (m_stack.empty())
            {
                assert(false);
                return;
            }

            assert(node == m_stack.top());

            m_stack.pop();
        }

    public:
#define GET_TAG_NODE_OR_RET(tag)  YamlNode node = GetTagNode(tag); if (node.IsNull()) return; 

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {            
            GET_TAG_NODE_OR_RET(tag);
            value = node.as<T>();
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.as<int>();
        }

        void inline Serialize(const char *tag, bool &value)
        {
            GET_TAG_NODE_OR_RET(tag);
            value = node.as<bool>();
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            GET_TAG_NODE_OR_RET(tag);
            str = node.as<std::string>();
        }

        template <typename T, int N>
        void Serialize(const char *tag, T(&array)[N])
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            int size = std::min(N, node.size());
            for (int i = 0, auto itr = node.begin(); i < size && itr != node.end(); i++, itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);
                Serialize("item", array[i]);
                EndObject(subNode);
            }
        }

        template <typename T, int N1, int N2>
        void Serialize(const char *tag, T(&array)[N1][N2])
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            int size = std::min(N, node.size());
            for (int i = 0, auto itr = node.begin(); i < size && itr != node.end(); i++, itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);
                Serialize("item", array[i]);
                EndObject(subNode);
            }
        }

        template <typename T, int N1, int N2, int N3>
        void Serialize(const char *tag, T(&array)[N1][N2][N3])
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            int size = std::min(N, node.size());
            for (int i = 0, auto itr = node.begin(); i < size && itr != node.end(); i++, itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);
                Serialize("item", array[i]);
                EndObject(subNode);
            }
        }

        template <typename T>
        typename std::enable_if<std::is_class<T>::value, void>::type
            Serialize(const char *tag, T &obj)
        {
            GET_TAG_NODE_OR_RET(tag);

            StartObject(node);
            obj.Serialize(*this);
            EndObject(node);
        }

        template <typename T>
        void SerializeArrayVector(const char *tag, std::vector<T> &vec)
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            size = std::min(vec.size(), node.size());
            for (int i = 0, auto itr = node.begin(); i < size && itr != node.end(); i++, itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);

                T obj;
                Serialize("item", obj);
                vec[i] = obj;

                EndObject(subNode);
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::vector<T> &vec)
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            for (auto itr = node.begin(); itr != node.end(); itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);

                T obj;
                Serialize("item", obj);
                vec.emplace_back(obj);

                EndObject(subNode);
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::list<T> &ls)
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            for (auto itr = node.begin(); itr != node.end(); itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);

                T obj;
                Serialize("item", obj);
                ls.emplace_back(obj);

                EndObject(subNode);
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::stack<T> &st)
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            for (auto itr = node.begin(); itr != node.end(); itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);

                T obj;
                Serialize("item", obj);
                st.emplace(obj);

                EndObject(subNode);
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::deque<T> &deq)
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            for (auto itr = node.begin(); itr != node.end(); itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);

                T obj;
                Serialize("item", obj);
                deq.emplace_back(obj);

                EndObject(subNode);
            }
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            for (auto itr = node.begin(); itr != node.end(); itr++)
            {
                YamlNode &subNode = *itr;
                if (!subNode["key"].IsNull() && !subNode["value"].IsNull())
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
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            for (auto itr = node.begin(); itr != node.end(); itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);

                T obj;
                Serialize("item", obj);
                set.insert(obj);

                EndObject(subNode);
            }
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
        {
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            for (auto itr = node.begin(); itr != node.end(); itr++)
            {
                YamlNode subNode = *itr;
                if (!subNode["key"].IsNull() && !subNode["value"].IsNull())
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
            GET_TAG_NODE_OR_RET(tag);
            assert(node.Type() == YAML::NodeType::Sequence);

            for (auto itr = node.begin(); itr != node.end(); itr++)
            {
                YamlNode &subNode = *itr;

                StartObject(subNode);

                T obj;
                Serialize("item", obj);
                set.insert(obj);

                EndObject(subNode);
            }
        }

#undef GET_TAG_NODE_OR_RET

    private:
        std::stack<YAML::Node>      m_stack;
    };
}