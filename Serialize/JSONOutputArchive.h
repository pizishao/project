#pragma once

#include "SerializeHeader.h"
#include "SerializeTraits.h"

class JsonOutPutArchive
{
public:
    JsonOutPutArchive() :
        writer(s)
    {

    }

    ~JsonOutPutArchive(){}

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
        writer.String(tag);

        writer.StartArray();

        for (int i = 0; i < N; i++)
        {
            writer.StartObject();
            Serialize("item", array[i]);
            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename T, int N1, int N2>
    void Serialize(const char *tag, T(&array)[N1][N2])
    {
        writer.String(tag);

        writer.StartArray();

        for (int i = 0; i < N1; i++)
        {
            writer.StartObject();
            Serialize("item", array[i]);
            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename T, int N1, int N2, int N3>
    void Serialize(const char *tag, T(&array)[N1][N2][N3])
    {
        writer.String(tag);

        writer.StartArray();

        for (int i = 0; i < N1; i++)
        {
            writer.StartObject();
            Serialize("item", array[i]);
            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    Serialize(const char *tag, T &obj)
    {
        writer.String(tag);

        writer.StartObject();
        obj.Serialize(*this);
        writer.EndObject();
    }    

    template <typename T>
    void Serialize(const char *tag, std::vector<T> &vec)
    {
        writer.String(tag);

        writer.StartArray();

        for (auto &pos : vec)
        {
            writer.StartObject();
            Serialize("item", pos);
            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename T>
    void Serialize(const char *tag, std::list<T> &ls)
    {
        writer.String(tag);

        writer.StartArray();

        for (auto &pos : ls)
        {
            writer.StartObject();
            Serialize("item", pos);
            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename T>
    void Serialize(const char *tag, std::stack<T> &st)
    {
        writer.String(tag);

        std::stack<T> tmpStack;
        while (!st.empty())
        {
            tmpStack.emplace(st.top());
            st.pop();
        }

        writer.StartArray();

        while (!tmpStack.empty())
        {
            writer.StartObject();

            Serialize("item", tmpStack.top());
            st.emplace(tmpStack.top());
            tmpStack.pop();

            writer.EndObject();                        
        }

        writer.EndArray();
    }

    template <typename T>
    void Serialize(const char *tag, std::deque<T> &deq)
    {
        writer.String(tag);

        std::deque<T> tmpDeq;       
        while (!deq.empty())
        {
            tmpDeq.emplace_back(deq.front());
            deq.pop_front();
        }

        writer.StartArray();

        while (!tmpDeq.empty())
        {
            writer.StartObject();

            Serialize("item", tmpDeq.front());
            deq.emplace_back(tmpDeq.front());
            tmpDeq.pop_front();

            writer.EndObject();            
        }

        writer.EndArray();
    }

    template <typename _Kty, typename _Ty>
    void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
    {
        writer.String(tag);

        writer.StartArray();

        for (auto &pos : mp)
        {
            writer.StartObject();

            Serialize("key", pos.first);
            Serialize("value", pos.second);

            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename T>
    void Serialize(const char *tag, std::set<T> &set)
    {
        writer.String(tag);

        writer.StartArray();

        for (auto &pos : set)
        {
            writer.StartObject();
            Serialize("item", (T &)pos);
            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename _Kty, typename _Ty>
    void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
    {
        writer.String(tag);

        writer.StartArray();

        for (auto &pos : mp)
        {
            writer.StartObject();

            Serialize("key", pos.first);
            Serialize("value", pos.second);

            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename T>
    void Serialize(const char *tag, std::unordered_set<T> &set)
    {
        writer.String(tag);

        writer.StartArray();

        for (auto &pos : set)
        {
            writer.StartObject();
            Serialize("item", (T &)pos);
            writer.EndObject();
        }

        writer.EndArray();
    }

    template <typename T>
    void operator << (T &obj)
    {
        writer.StartObject();
        obj.Serialize(*this);
        writer.EndObject();
    }

    std::string GetJsonText()
    {
        return s.GetString();
    }

private:
    rapidjson::StringBuffer                     s;
    rapidjson::Writer<rapidjson::StringBuffer>  writer;
};