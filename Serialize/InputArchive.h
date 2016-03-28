#pragma once

namespace Serialization
{
    template <typename Archive, typename T>
    void Serialize(Archive &ar, T &obj);

    template<typename archive, bool intrusive = true>
    class InputArchive
    {
        static const bool is_intrusive = intrusive;

    public:
        InputArchive(){}
        ~InputArchive(){}

#define CHECK_TAG_NODE_EXIST_OR_RET(tag) if (!m_outArchive.CheckTagNodeExist(tag)) return;

    public:
        bool Load(const std::string text)
        {
            return m_outArchive.Load(text);
        }

        bool LoadFromFile(std::string filename)
        {
            return m_outArchive.LoadFromFile(filename);
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
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);
            int size = std::min(count, N);

            for (int i = 0; i < size; i++)
            {
                m_outArchive.StartItem(i);
                Serialize("item", array[i]);
                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T, int N1, int N2>
        void Serialize(const char *tag, T(&array)[N1][N2])
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);
            int size = std::min(count, N1);

            for (int i = 0; i < size; i++)
            {
                m_outArchive.StartItem(i);
                Serialize("item", array[i]);
                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T, int N1, int N2, int N3>
        void Serialize(const char *tag, T(&array)[N1][N2][N3])
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);
            int size = std::min(count, N1);

            for (int i = 0; i < size; i++)
            {
                m_outArchive.StartItem(i);
                Serialize("item", array[i]);
                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template<class T>
        static auto FakeSerializeClass(const char *tag, T &obj, long a)
            -> decltype(intrusive_if<InputArchive::is_intrusive, std::is_class<T>::value>::yes_class, 
                intrusive_if<InputArchive::is_intrusive, std::is_class<T>::value>::yes, void())
        {
            return;
        }

        template<class T>
        static auto FakeSerializeClass(const char *tag, T &obj, int a)
            -> decltype(intrusive_if<InputArchive::is_intrusive, std::is_class<T>::value>::yes_class, 
                intrusive_if<InputArchive::is_intrusive, std::is_class<T>::value>::no, void())
        {
            return;
        }

        template<class T>
        auto SerializeClass(const char *tag, T &obj, int a)
            -> decltype(intrusive_if<InputArchive::is_intrusive, std::is_class<T>::value>::yes_class, 
                intrusive_if<InputArchive::is_intrusive, std::is_class<T>::value>::no, void())
        {
            if (!tag)
            {
                Serialization::Serialize(*this, obj);
            }
            else
            {
                CHECK_TAG_NODE_EXIST_OR_RET(tag);

                m_outArchive.StartObject(tag);
                Serialization::Serialize(*this, obj);
                m_outArchive.EndObject(tag);
            }
        }

        template<class T>
        auto SerializeClass(const char *tag, T &obj, long a)
            -> decltype(intrusive_if<is_intrusive, std::is_class<T>::value>::yes_class, 
                intrusive_if<is_intrusive, std::is_class<T>::value>::yes, void())
        {   
            if (!tag)
            {
                obj.Serialize(*this);
            }
            else
            {
                CHECK_TAG_NODE_EXIST_OR_RET(tag);

                m_outArchive.StartObject(tag);
                obj.Serialize(*this);
                m_outArchive.EndObject(tag);
            }            
        }

        template <typename T>
        auto Serialize(const char *tag, T &obj)
            -> decltype(FakeSerializeClass(tag, obj, 0))
        {            
            SerializeClass(tag, obj, 0);
        }

        template <typename T>
        void SerializeArrayVector(const char *tag, std::vector<T> &vec)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            int size = std::min((int)vec.size(), count);
            for (int i = 0; i < size; i++)
            {
                m_outArchive.StartItem(i);

                T obj;
                Serialize("item", obj);
                vec[i] = obj;

                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::vector<T> &vec)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            for (int i = 0; i < count; i++)
            {
                m_outArchive.StartItem(i);

                T obj;
                Serialize("item", obj);
                vec.emplace_back(obj);

                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::list<T> &ls)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            for (int i = 0; i < count; i++)
            {
                m_outArchive.StartItem(i);

                T obj;
                Serialize("item", obj);
                ls.emplace_back(obj);

                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::stack<T> &st)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            for (int i = 0; i < count; i++)
            {
                m_outArchive.StartItem(i);

                T obj;
                Serialize("item", obj);
                st.emplace(obj);

                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::deque<T> &deq)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            for (int i = 0; i < count; i++)
            {
                m_outArchive.StartItem(i);

                T obj;
                Serialize("item", obj);
                deq.emplace_back(obj);

                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            for (int i = 0; i < count; i++)
            {
                if (m_outArchive.ItemHasTag(i, "key")
                    && m_outArchive.ItemHasTag(i, "value"))
                {
                    _Kty key;
                    _Ty  value;

                    m_outArchive.StartItem(i);

                    Serialize("key", key);
                    Serialize("value", value);
                    mp.insert({ key, value });

                    m_outArchive.EndItem();
                }
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::set<T> &set)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            for (int i = 0; i < count; i++)
            {
                m_outArchive.StartItem(i);

                T obj;
                Serialize("item", obj);
                set.insert(obj);

                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            for (int i = 0; i < count; i++)
            {
                if (m_outArchive.ItemHasTag(i, "key")
                    && m_outArchive.ItemHasTag(i, "value"))
                {
                    _Kty key;
                    _Ty  value;

                    m_outArchive.StartItem(i);

                    Serialize("key", key);
                    Serialize("value", value);
                    mp.insert({ key, value });

                    m_outArchive.EndItem();
                }
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::unordered_set<T> &set)
        {
            CHECK_TAG_NODE_EXIST_OR_RET(tag);

            int count = m_outArchive.StartArray(tag);

            for (int i = 0; i < count; i++)
            {
                m_outArchive.StartItem(i);

                T obj;
                Serialize("item", obj);
                set.insert(obj);

                m_outArchive.EndItem();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        auto operator >> (T &obj)
            -> decltype(FakeSerializeClass(nullptr, obj, 0))
        {
            SerializeClass(nullptr, obj, 0);
        }

#undef CHECK_TAG_NODE_EXIST_OR_RET

    private:
        archive m_outArchive;
    };
}