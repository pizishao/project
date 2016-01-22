#pragma once

#include <fstream>
#include <sstream>

#include "SerializeHeader.h"
#include "SerializeTraits.h"

#include "rapidjson/document.h"

#ifdef  WIN32
#pragma warning(disable:4146)
#endif //  WIN32

class JsonInPutArchive
{
public:
    JsonInPutArchive(){}
    ~JsonInPutArchive(){}

private:
    rapidjson::Value* GetTagValue(const char *tag)
    {
        if (m_stack.empty())
        {
            assert(false);
            return nullptr;
        }

        rapidjson::Value &jVal = *m_stack.top();

        if (jVal.HasMember(tag))
        {
            return &(jVal[tag]);
        }
        else
        {
            return nullptr;
        }
    }

    void StartObject(rapidjson::Value *val)
    {
        m_stack.push(val);
    }

    void EndObject(rapidjson::Value *val)
    {
        if (m_stack.empty())
        {
            assert(false);
            return;
        }

        if (m_stack.top() != val)
        {
            assert(false);
            return;
        }

        m_stack.pop();
    }

public:

#define GET_JVALUE_OR_RET(tag) rapidjson::Value *jVal = GetTagValue(tag); if (!jVal) return;

    bool Load(const std::string jsontext)
    {
        m_doc.Parse<0>(jsontext.c_str());

        if (m_doc.HasParseError())
        {
            assert(false);
            return false;
        }

        m_stack.push(&m_doc);

        return true;
    }

    bool LoadFromFile(std::string filename)
    {
        std::ifstream in(filename.c_str());

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

    template <typename T>
    typename std::enable_if<is_signedBigInt<T>::value, void>::type
    inline Serialize(const char *tag, T &value)
    {
        GET_JVALUE_OR_RET(tag)
            
        value = jVal->GetInt64();
    }

    template <typename T>
    typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
    inline Serialize(const char *tag, T &value)
    {
        GET_JVALUE_OR_RET(tag)

        value = jVal->GetUint64();
    }

    template <typename T>
    typename std::enable_if<is_signedSmallInt<T>::value, void>::type
    inline Serialize(const char *tag, T &value)
    {
        GET_JVALUE_OR_RET(tag)

        value = jVal->GetInt();
    }

    template <typename T>
    typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
    inline   Serialize(const char *tag, T &value)
    {
        GET_JVALUE_OR_RET(tag)

        value = jVal->GetUint();
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
    inline Serialize(const char *tag, T& value)
    {
        GET_JVALUE_OR_RET(tag)

        value = jVal->GetDouble();
    }    

    void inline Serialize(const char *tag, bool &value)
    {
        GET_JVALUE_OR_RET(tag)

        value = jVal->GetBool();
    }

    void inline Serialize(const char *tag, std::string &str)
    {
        GET_JVALUE_OR_RET(tag)
            
        str = jVal->GetString();
    }

    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
        Serialize(const char* tag, T& value)
    {
        GET_JVALUE_OR_RET(tag)
       
        value = (T)jVal->GetInt();      
    }

    template <typename T, int N>
    void Serialize(const char *tag, T(&array)[N])
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        int size = std::min(N, arrayValue.Size());
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);
            Serialize("item", array[i]);
            EndObject(&arrayValue[i]);
        }
    }

    template <typename T, int N1, int N2>
    void Serialize(const char *tag, T(&array)[N1][N2])
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        int size = std::min(N1, arrayValue.Size());
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);
            Serialize("item", array[i]);
            EndObject(&arrayValue[i]);
        }
    }

    template <typename T, int N1, int N2, int N3>
    void Serialize(const char *tag, T(&array)[N1][N2][N3])
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        int size = std::min(N1, arrayValue.Size());
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);
            Serialize("item", array[i]);
            EndObject(&arrayValue[i]);
        }
    }    

    template <typename T> 
    typename std::enable_if<std::is_class<T>::value, void>::type
        Serialize(const char *tag, T &obj)
    {
        GET_JVALUE_OR_RET(tag)

        StartObject(jVal);
        obj.Serialize(*this);
        EndObject(jVal);
    }

    template <typename T>
    void SerializeArrayVector(const char *tag, std::vector<T> &vec)
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        size = std::min(vec.size(), arrayValue.Size());
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);

            T obj;
            Serialize("item", obj);
            vec[i] = obj;

            EndObject(&arrayValue[i]);
        }
    }

    template <typename T>
    void Serialize(const char *tag, std::vector<T> &vec)
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());
            
        int size = arrayValue.Size();
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);

            T obj;
            Serialize("item", obj);
            vec.emplace_back(obj);

            EndObject(&arrayValue[i]);
        }                          
    }

    template <typename T>
    void Serialize(const char *tag, std::list<T> &ls)
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        int size = arrayValue.Size();
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);

            T obj;
            Serialize("item", obj);
            ls.emplace_back(obj);

            EndObject(&arrayValue[i]);
        }
    }

    template <typename T>
    void Serialize(const char *tag, std::stack<T> &st)
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        int size = arrayValue.Size();
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);

            T obj;
            Serialize("item", obj);
            st.emplace(obj);

            EndObject(&arrayValue[i]);
        }
    }

    template <typename T>
    void Serialize(const char *tag, std::deque<T> &deq)
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        int size = arrayValue.Size();
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);

            T obj;
            Serialize("item", obj);
            deq.emplace_back(obj);

            EndObject(&arrayValue[i]);
        }
    }

    template <typename _Kty, typename _Ty>
    void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
    {
        GET_JVALUE_OR_RET(tag)
        
        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        for (std::size_t i = 0; i < arrayValue.Size(); i++)
        {
            if (arrayValue[i].HasMember("key") && arrayValue[i].HasMember("value"))
            {
                _Kty key;
                _Ty  value;

                StartObject(&arrayValue[i]);

                Serialize("key", key);
                Serialize("value", value);
                mp.insert({ key, value });

                EndObject(&arrayValue[i]);
            }
            else
            {
                ;
            }
        }        
    }

    template <typename T>
    void Serialize(const char *tag, std::set<T> &set)
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        int size = arrayValue.Size();
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);

            T obj;
            Serialize("item", obj);
            set.insert(obj);

            EndObject(&arrayValue[i]);
        }
    }

    template <typename _Kty, typename _Ty>
    void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
    {
        GET_JVALUE_OR_RET(tag)
            
        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        for (std::size_t i = 0; i < arrayValue.Size(); i++)
        {
            if (arrayValue[i].HasMember("key") && arrayValue[i].HasMember("value"))
            {
                _Kty key;
                _Ty  value;

                StartObject(&arrayValue[i]);

                Serialize("key", key);
                Serialize("value", value);
                mp.insert({ key, value });

                EndObject(&arrayValue[i]);
            }
            else
            {
                ;
            }
        }       
    }

    template <typename T>
    void Serialize(const char *tag, std::unordered_set<T> &set)
    {
        GET_JVALUE_OR_RET(tag)

        rapidjson::Value &arrayValue = *jVal;
        assert(arrayValue.IsArray());

        int size = arrayValue.Size();
        for (int i = 0; i < size; i++)
        {
            StartObject(&arrayValue[i]);

            T obj;
            Serialize("item", obj);
            set.insert(obj);

            EndObject(&arrayValue[i]);
        }
    }

#undef GET_JVALUE_OR_RET

private:
    std::stack<rapidjson::Value *> m_stack;
    rapidjson::Document m_doc;
};
