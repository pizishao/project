#pragma once

#include "traits.h"
#include "common.h"

namespace Serialization
{
    template<typename archive, bool intrusive>
    class OutputArchive
    {
        static const bool is_intrusive = intrusive;

    public:
        OutputArchive(){}
        ~OutputArchive(){}

    public:
        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            m_outArchive.Serialize(tag, value);
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            m_outArchive.Serialize(tag, value);
        }

        void inline Serialize(const char *tag, bool value)
        {
            m_outArchive.Serialize(tag, value);
        }

        void inline Serialize(const char *tag, std::string str)
        {
            m_outArchive.Serialize(tag, str);
        }

        template <typename T, int N>
        void Serialize(const char *tag, T(&array)[N])
        {
            m_outArchive.StartArray(tag);

            for (int i = 0; i < N; i++)
            {
                m_outArchive.Serialize("item", array[i]);
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T, int N1, int N2>
        void Serialize(const char *tag, T(&array)[N1][N2])
        {
            m_outArchive.StartArray(tag);

            for (int i = 0; i < N1; i++)
            {
                m_outArchive.Serialize("item", array[i]);
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T, int N1, int N2, int N3>
        void Serialize(const char *tag, T(&array)[N1][N2][N3])
        {
            m_outArchive.StartArray(tag);

            for (int i = 0; i < N1; i++)
            {
                m_outArchive.Serialize("item", array[i]);
            }

            m_outArchive.EndArray(tag);
        }

        template<class T>
        auto SerializeClass(const char *tag, T &obj, int a)
            -> decltype(intrusive_if<is_intrusive, std::is_class<T>::value>::yes_class, intrusive_if<is_intrusive, std::is_class<T>::value>::no, void())
        {
            m_outArchive.StartObject(tag);
            Serialization::Serialize(*this, obj);
            m_outArchive.EndObject(tag);
        }

        template<class T>
        auto SerializeClass(const char *tag, T &obj, long a)
            -> decltype(intrusive_if<is_intrusive, std::is_class<T>::value>::yes_class, intrusive_if<is_intrusive, std::is_class<T>::value>::yes, void())
        {
            m_outArchive.StartObject(tag);
            obj.Serialize(*this);
            m_outArchive.EndObject(tag);
        }

        template <typename T>
        auto Serialize(const char *tag, T &obj)
            -> decltype(SerializeClass(tag, obj, 0))
        {
            SerializeClass(tag, obj, 0);
        }

        template <typename T>
        void Serialize(const char *tag, std::vector<T> &vec)
        {
            m_outArchive.StartArray(tag);

            for (auto &pos : vec)
            {
                m_outArchive.StartObject(nullptr);
                Serialize("item", pos);
                m_outArchive.EndObject(nullptr);
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::list<T> &ls)
        {
            m_outArchive.StartArray(tag);

            for (auto &pos : ls)
            {
                m_outArchive.StartObject(nullptr);
                Serialize("item", pos);
                m_outArchive.EndObject(nullptr);
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::stack<T> &st)
        {
            std::stack<T> tmpStack;
            while (!st.empty())
            {
                tmpStack.emplace(st.top());
                st.pop();
            }

            m_outArchive.StartArray(tag);

            while (!tmpStack.empty())
            {
                m_outArchive.StartObject(nullptr);
                Serialize("item", tmpStack.top());
                m_outArchive.EndObject(nullptr);

                st.emplace(tmpStack.top());
                tmpStack.pop();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::deque<T> &deq)
        {
            std::deque<T> tmpDeq;
            while (!deq.empty())
            {
                tmpDeq.emplace_back(deq.front());
                deq.pop_front();
            }

            m_outArchive.StartArray(tag);

            while (!tmpDeq.empty())
            {
                m_outArchive.StartObject(nullptr);
                Serialize("item", tmpDeq.front());
                m_outArchive.EndObject(nullptr);

                deq.emplace_back(tmpDeq.front());
                tmpDeq.pop_front();
            }

            m_outArchive.EndArray(tag);
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
        {
            m_outArchive.StartArray(tag);

            for (auto &pos : mp)
            {
                m_outArchive.StartObject(nullptr);
                Serialize("key", pos.first);
                Serialize("value", pos.second);
                m_outArchive.EndObject(nullptr);
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::set<T> &set)
        {
            m_outArchive.StartArray(tag);

            for (auto &pos : set)
            {
                m_outArchive.StartObject(nullptr);
                Serialize("item", (T &)pos);
                m_outArchive.EndObject(nullptr);
            }

            m_outArchive.EndArray(tag);
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
        {
            m_outArchive.StartArray(tag);

            for (auto &pos : mp)
            {
                m_outArchive.StartObject(nullptr);
                Serialize("key", pos.first);
                Serialize("value", pos.second);
                m_outArchive.EndObject(nullptr);
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        void Serialize(const char *tag, std::unordered_set<T> &set)
        {
            m_outArchive.StartArray(tag);

            for (auto &pos : set)
            {
                m_outArchive.StartObject(nullptr);
                Serialize("item", (T &)pos);
                m_outArchive.EndObject(nullptr);
            }

            m_outArchive.EndArray(tag);
        }

        template <typename T>
        auto operator << (T &obj)
            -> decltype(SerializeClass(nullptr, obj, 0))
        {
            SerializeClass(nullptr, obj, 0);
        }

        std::string c_str()
        {
            return m_outArchive.c_str();
        }

    private:
        archive m_outArchive;
    };
}