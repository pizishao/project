#pragma once

namespace Serialization
{
    template<typename archive, bool intrusive = true>
    class InputArchive
    {
        static const bool is_intrusive = intrusive;

    public:
        InputArchive(){}
        ~InputArchive(){}

#define GET_TAG_NODE_OR_RET(tag) Node node = m_outArchive.GetTagNode(tag); if (node.elem.IsNull()) return;

    public:
        bool Load(const std::string text)
        {
            return m_outArchive.Load(text);
        }

        bool LoadFromFile(std::string filename)
        {
            return m_outArchive.LoadFromFile(text);
        }

        template <typename T>
        auto operator >> (T &obj)
            -> decltype(SerializeClass(nullptr, obj, 0))
        {
            SerializeClass(nullptr, obj, 0);
        }

        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            inline Serialize(const char *tag, T &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            inline   Serialize(const char *tag, T &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            m_outArchive.Serialize(tag, value);
        }

        void inline Serialize(const char *tag, bool &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        void inline Serialize(const char *tag, std::string &str)
        {
            m_outArchive.Serialize(tag, str);
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T, int N>
        void Serialize(const char *tag, T(&array)[N])
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            int size = std::min(N, children.size());
            for (int i = 0; i < size; i++)
            {
                m_outArchive.m_outArchive.StartObject(children[i]);
                Serialize("item", array[i]);
                m_outArchive.m_outArchive.EndObject(children[i]);
            }
        }

        template <typename T, int N1, int N2>
        void Serialize(const char *tag, T(&array)[N1][N2])
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            int size = std::min(N1, children.size());
            for (int i = 0; i < size; i++)
            {
                m_outArchive.m_outArchive.StartObject(children[i]);
                Serialize("item", array[i]);
                m_outArchive.m_outArchive.EndObject(children[i]);
            }
        }

        template <typename T, int N1, int N2, int N3>
        void Serialize(const char *tag, T(&array)[N1][N2][N3])
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            int size = std::min(N1, children.size());
            for (int i = 0; i < size; i++)
            {
                m_outArchive.m_outArchive.StartObject(children[i]);
                Serialize("item", array[i]);
                m_outArchive.EndObject(children[i]);
            }
        }

        template<class T>
        auto SerializeClass(const char *tag, T &obj, int a)
            -> decltype(intrusive_if<is_intrusive, std::is_class<T>::value>::yes_class, intrusive_if<is_intrusive, std::is_class<T>::value>::no, void())
        {   
            if (!tag)
            {
                Serialization::Serialize(*this, obj);
            }
            else
            {
                GET_TAG_NODE_OR_RET(tag);

                m_outArchive.StartObject(node.elem);
                Serialization::Serialize(*this, obj);
                m_outArchive.EndObject(node.elem);
            }           
        }

        template<class T>
        auto SerializeClass(const char *tag, T &obj, long a)
            -> decltype(intrusive_if<is_intrusive, std::is_class<T>::value>::yes_class, intrusive_if<is_intrusive, std::is_class<T>::value>::yes, void())
        {   
            if (!tag)
            {
                obj.Serialize(*this);
            }
            else
            {
                GET_TAG_NODE_OR_RET(tag);

                m_outArchive.StartObject(node.elem);
                obj.Serialize(*this);
                m_outArchive.EndObject(node.elem);
            }            
        }

        template <typename T>
        auto Serialize(const char *tag, T &obj)
            -> decltype(SerializeClass(tag, obj, 0))
        {            
            SerializeClass(tag, obj, 0);
        }

        template <typename T>
        void SerializeArrayVector(const char *tag, std::vector<T> &vec)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            int size = std::min(vec.size(), children.size());
            for (int i = 0; i < size; i++)
            {
                m_outArchive.StartObject(children[i]);

                T obj;
                Serialize("item", obj);
                vec[i] = obj;

                m_outArchive.EndObject(children[i]);
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::vector<T> &vec)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            for (size_t i = 0; i < children.size(); i++)
            {
                m_outArchive.StartObject(children[i]);

                T obj;
                Serialize("item", obj);
                vec.emplace_back(obj);

                m_outArchive.EndObject(children[i]);
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::list<T> &ls)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            for (size_t i = 0; i < children.size(); i++)
            {
                m_outArchive.StartObject(children[i]);

                T obj;
                Serialize("item", obj);
                ls.emplace_back(obj);

                m_outArchive.EndObject(children[i]);
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::stack<T> &st)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            for (size_t i = 0; i < children.size(); i++)
            {
                m_outArchive.StartObject(children[i]);

                T obj;
                Serialize("item", obj);
                st.emplace(obj);

                m_outArchive.EndObject(children[i]);
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::deque<T> &deq)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            for (size_t i = 0; i < children.size(); i++)
            {
                m_outArchive.StartObject(children[i]);

                T obj;
                Serialize("item", obj);
                deq.emplace_back(obj);

                m_outArchive.EndObject(children[i]);
            }
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            for (size_t i = 0; i < children.size(); i++)
            {
                if (m_outArchive.HasMember("key", children[i]) 
                    && m_outArchive.HasMember("value", children[i]))
                {
                    _Kty key;
                    _Ty  value;

                    m_outArchive.StartObject(children[i]);

                    Serialize("key", key);
                    Serialize("value", value);
                    mp.insert({ key, value });

                    m_outArchive.EndObject(children[i]);
                }
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::set<T> &set)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            for (size_t i = 0; i < children.size(); i++)
            {
                m_outArchive.StartObject(children[i]);

                T obj;
                Serialize("item", obj);
                set.insert(obj);

                m_outArchive.EndObject(children[i]);
            }
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            for (size_t i = 0; i < children.size(); i++)
            {
                if (m_outArchive.HasMember("key", children[i]) 
                    && m_outArchive.HasMember("value", children[i]))
                {
                    _Kty key;
                    _Ty  value;

                    m_outArchive.StartObject(children[i]);

                    Serialize("key", key);
                    Serialize("value", value);
                    mp.insert({ key, value });

                    m_outArchive.EndObject(children[i]);
                }
            }
        }

        template <typename T>
        void Serialize(const char *tag, std::unordered_set<T> &set)
        {
            GET_TAG_NODE_OR_RET(tag);

            auto &children = node.children;

            for (size_t i = 0; i < children.size(); i++)
            {
                m_outArchive.StartObject(children[i]);

                T obj;
                Serialize("item", obj);
                set.insert(obj);

                m_outArchive.EndObject(children[i]);
            }
        }

#undef GET_TAG_NODE_OR_RET

    private:
        archive m_outArchive;
    };
}