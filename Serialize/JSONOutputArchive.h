#pragma once

#include "SerializeHeader.h"
#include "SerializeTraits.h"
#include "RapidjsonWrap.h"

namespace Serialization
{
    class JsonOutPutArchive
    {
    public:
        JsonOutPutArchive() : writer(s){}
        ~JsonOutPutArchive(){}

    private:
        void StartArray(const char *tag)
        {
            writer.String(tag);
            writer.StartArray();
        }

        void EndArray()
        {
            writer.EndArray();
        }

        void StartObject(const char *tag)
        {
            if (tag)
            {
                writer.String(tag);
                writer.StartObject();
            }
            else
            {
                writer.StartObject();
            }
        }

        void EndObject()
        {
            writer.EndObject();
        }

    public:

        template <typename T>
        typename std::enable_if<is_signedBigInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer.String(tag);
            writer.Int64(value);
        }

        template <typename T>
        typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer.String(tag);
            writer.Uint64(value);
        }

        template <typename T>
        typename std::enable_if<is_signedSmallInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer.String(tag);
            writer.Int(value);
        }

        template <typename T>
        typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
            Serialize(const char *tag, T &value)
        {
            writer.String(tag);
            writer.UInt(value);
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
            inline Serialize(const char *tag, T& value)
        {
            writer.String(tag);
            writer.Double(value);
        }

        template<typename T>
        typename std::enable_if<std::is_enum<T>::value, void>::type
            Serialize(const char* tag, T& value)
        {
            writer.String(tag);
            writer.Int((int)value);
        }

        void inline Serialize(const char *tag, bool value)
        {
            writer.String(tag);
            writer.Bool(value);
        }

        void inline Serialize(const char *tag, std::string str)
        {
            writer.String(tag);
            writer.String(str.c_str());
        }

        template <typename T, int N>
        void Serialize(const char *tag, T(&array)[N])
        {
            StartArray(tag);

            for (int i = 0; i < N; i++)
            {
                StartObject(nullptr);
                Serialize("item", array[i]);
                EndObject();
            }

            EndArray();
        }

        template <typename T, int N1, int N2>
        void Serialize(const char *tag, T(&array)[N1][N2])
        {
            StartArray(tag);

            for (int i = 0; i < N1; i++)
            {
                StartObject(nullptr);
                Serialize("item", array[i]);
                EndObject();
            }

            EndArray();
        }

        template <typename T, int N1, int N2, int N3>
        void Serialize(const char *tag, T(&array)[N1][N2][N3])
        {
            StartArray(tag);

            for (int i = 0; i < N1; i++)
            {
                StartObject(nullptr);
                Serialize("item", array[i]);
                EndObject();
            }

            EndArray();
        }

        template <typename T>
        typename std::enable_if<std::is_class<T>::value, void>::type
            Serialize(const char *tag, T &obj)
        {
            StartObject(tag);
            obj.Serialize(*this);
            EndObject();
        }

        template <typename T>
        void Serialize(const char *tag, std::vector<T> &vec)
        {
            StartArray(tag);

            for (auto &pos : vec)
            {
                StartObject(nullptr);
                Serialize("item", pos);
                EndObject();
            }

            EndArray();
        }

        template <typename T>
        void Serialize(const char *tag, std::list<T> &ls)
        {
            StartArray(tag);

            for (auto &pos : ls)
            {
                StartObject(nullptr);
                Serialize("item", pos);
                EndObject();
            }

            EndArray();
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

            StartArray(tag);

            while (!tmpStack.empty())
            {
                StartObject(nullptr);
                Serialize("item", tmpStack.top());
                EndObject();

                st.emplace(tmpStack.top());
                tmpStack.pop();
            }

            EndArray();
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

            StartArray(tag);

            while (!tmpDeq.empty())
            {
                StartObject(nullptr);
                Serialize("item", tmpDeq.front());
                EndObject();

                deq.emplace_back(tmpDeq.front());
                tmpDeq.pop_front();
            }

            EndArray();
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
        {
            StartArray(tag);

            for (auto &pos : mp)
            {
                StartObject(nullptr);
                Serialize("key", pos.first);
                Serialize("value", pos.second);
                EndObject();
            }

            EndArray();
        }

        template <typename T>
        void Serialize(const char *tag, std::set<T> &set)
        {
            StartArray(tag);

            for (auto &pos : set)
            {
                StartObject(nullptr);
                Serialize("item", (T &)pos);
                EndObject();
            }

            EndArray();
        }

        template <typename _Kty, typename _Ty>
        void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
        {
            StartArray(tag);

            for (auto &pos : mp)
            {
                StartObject(nullptr);
                Serialize("key", pos.first);
                Serialize("value", pos.second);
                EndObject();
            }

            EndArray();
        }

        template <typename T>
        void Serialize(const char *tag, std::unordered_set<T> &set)
        {
            StartArray(tag);

            for (auto &pos : set)
            {
                StartObject(nullptr);
                Serialize("item", (T &)pos);
                EndObject();
            }

            EndArray();
        }

        template <typename T>
        void operator << (T &obj)
        {
            StartObject(nullptr);
            obj.Serialize(*this);
            EndObject();
        }

        std::string GetJsonText()
        {
            return s.GetString();
        }

    private:
        rapidjson::StringBuffer                     s;
        rapidjson::Writer<rapidjson::StringBuffer>  writer;
    };
}