#pragma once

#include <fstream>
#include <sstream>

#include "SerializeHeader.h"
#include "SerializeTraits.h"

#include "TinyXML/tinyxml.h" 

class XmlInPutArchive
{
public:

    XmlInPutArchive(){}

    ~XmlInPutArchive(){}

    bool Load(const std::string xmltext)
    {
        m_doc.Parse(xmltext.c_str());
        if (m_doc.Error())
        {
            std::string errorDesc = m_doc.ErrorDesc();
            assert(false);
            return false;
        }

        m_stack.push(m_doc.RootElement());

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
    int GetElemChildCount(TiXmlElement* elem)
    {
        int count = 0;

        if (!elem)
        {
            return 0;
        }

        for (TiXmlElement *itemElem = elem->FirstChildElement(); 
            itemElem != nullptr; itemElem = itemElem->NextSiblingElement())
        {
            count++;
        }

        return count;
    }

    TiXmlElement* GetTagElem(const char *tag)
    {
        if (m_stack.empty())
        {
            assert(false);
            return nullptr;
        }

        TiXmlElement &parent = *m_stack.top();
        TiXmlElement *elem = parent.FirstChildElement(tag);

        return elem;
    }

    void StartObject(TiXmlElement *elem)
    {
        m_stack.push(elem);
    }

    void EndObject(TiXmlElement *elem)
    {
        if (m_stack.empty())
        {
            assert(false);
            return;
        }

        if (m_stack.top() != elem)
        {
            assert(false);
            return;
        }


        m_stack.pop();
    }

    void StartArray(TiXmlElement *elem)
    {
        StartObject(elem);
    }

    void EndArray(TiXmlElement *elem)
    {
        EndObject(elem);
    }
    
public:

#define GET_TAG_ELEM_OR_RET(tag)  TiXmlElement *elem = GetTagElem(tag); if (!elem) return; 

    template <typename T>
    typename std::enable_if<is_signedBigInt<T>::value, void>::type
        inline Serialize(const char *tag, T &value)
    {
        GET_TAG_ELEM_OR_RET(tag)

        value = (T)std::atoll(elem->GetText());        
    }

    template <typename T>
    typename std::enable_if<is_unsignedBigInt<T>::value, void>::type
        inline Serialize(const char *tag, T &value)
    {
        GET_TAG_ELEM_OR_RET(tag)

        value = (T)std::atoll(elem->GetText());
    }

    template <typename T>
    typename std::enable_if<is_signedSmallInt<T>::value, void>::type
        inline Serialize(const char *tag, T &value)
    {
        GET_TAG_ELEM_OR_RET(tag)

        value = (T)std::atoi(elem->GetText());
    }

    template <typename T>
    typename std::enable_if<is_unsignedSmallInt<T>::value, void>::type
        inline   Serialize(const char *tag, T &value)
    {
        GET_TAG_ELEM_OR_RET(tag)

        value = (T)std::atoi(elem->GetText());
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type
        inline Serialize(const char *tag, T& value)
    {
        GET_TAG_ELEM_OR_RET(tag)

        value = (T)std::atof(elem->GetText());
    }

    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
        Serialize(const char* tag, T& value)
    {
        GET_TAG_ELEM_OR_RET(tag)

        value = (T)std::atoi(elem->GetText());
    }

    void inline Serialize(const char *tag, bool &value)
    {
        GET_TAG_ELEM_OR_RET(tag)

        value = (bool)std::atoi(elem->GetText());
    }

    void inline Serialize(const char *tag, std::string &str)
    {
        GET_TAG_ELEM_OR_RET(tag)

        str = elem->GetText();      
    }   

    template <typename T, int N>
    void Serialize(const char *tag, T(&array)[N])
    {
        GET_TAG_ELEM_OR_RET(tag)
   
        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount && i < N; i++)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, array[i]);
        }

        EndArray(elem);
    }

    template <typename T, int N1, int N2>
    void Serialize(const char *tag, T(&array)[N1][N2])
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount && i < N1; i++)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, array[i]);
        }

        EndArray(elem);
    }

    template <typename T, int N1, int N2, int N3>
    void Serialize(const char *tag, T(&array)[N1][N2][N3])
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount && i < N1; i++)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, array[i]);
        }

        EndArray(elem);
    }

    template <typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
        Serialize(const char *tag, T &obj)
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartObject(elem);
        obj.Serialize(*this);
        EndObject(elem);
    }

    template <typename T>
    void SerializeArrayVector(const char *tag, std::vector<T> &vec)
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount && i < (int)vec.size(); i++)
        {
            T obj;
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, obj);
            vec[i] = obj;
        }

        EndArray(elem);
    }

    template <typename T>
    void Serialize(const char *tag, std::vector<T> &vec)
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount; i++)
        {
            T obj;
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, obj);
            vec.emplace_back(obj);
        }

        EndArray(elem);
    }

    template <typename T>
    void Serialize(const char *tag, std::list<T> &ls)
    {
        GET_TAG_ELEM_OR_RET(tag)
          
        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount; i++)
        {
            T obj;
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, obj);
            ls.emplace_back(obj);
        }

        EndArray(elem);
    }

    template <typename T>
    void Serialize(const char *tag, std::stack<T> &st)
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount; i++)
        {
            T obj;
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, obj);
            st.emplace(obj);
        }

        EndArray(elem);
    }

    template <typename T>
    void Serialize(const char *tag, std::deque<T> &deq)
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount; i++)
        {
            T obj;
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, obj);
            deq.emplace_back(obj);
        }          

        EndArray(elem);
    }

    template <typename _Kty, typename _Ty>
    void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
    {
        GET_TAG_ELEM_OR_RET(tag)
          
        for (TiXmlElement *itemElem = elem->FirstChildElement(); itemElem != nullptr; 
            itemElem = itemElem->NextSiblingElement())
        {
            TiXmlElement *keyElem = itemElem->FirstChildElement("key");
            TiXmlElement *valueElem = itemElem->FirstChildElement("value");

            if (keyElem && valueElem)
            {
                _Kty key;
                _Ty  value;

                StartObject(itemElem);

                Serialize("key", key);
                Serialize("value", value);
                mp.insert({ key, value });

                EndObject(itemElem);
            }
        }           
    }

    template <typename T>
    void Serialize(const char *tag, std::set<T> &set)
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount; i++)
        {
            T obj;
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, obj);
            set.insert(obj);
        }

        EndArray(elem);
    }

    template <typename _Kty, typename _Ty>
    void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
    {
        GET_TAG_ELEM_OR_RET(tag)

        for (TiXmlElement *itemElem = elem->FirstChildElement(); itemElem != nullptr;
            itemElem = itemElem->NextSiblingElement())
        {
            TiXmlElement *keyElem = itemElem->FirstChildElement("key");
            TiXmlElement *valueElem = itemElem->FirstChildElement("value");

            if (keyElem && valueElem)
            {
                _Kty key;
                _Ty  value;

                StartObject(itemElem);

                Serialize("key", key);
                Serialize("value", value);
                mp.insert({ key, value });

                EndObject(itemElem);
            }
        }
    }

    template <typename T>
    void Serialize(const char *tag, std::unordered_set<T> &set)
    {
        GET_TAG_ELEM_OR_RET(tag)

        StartArray(elem);

        int childCount = GetElemChildCount(elem);
        for (int i = 0; i < childCount; i++)
        {
            T obj;
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, obj);
            set.insert(obj);
        }

        EndArray(elem);
    }

#undef GET_TAG_ELEM_OR_RET

private:
    TiXmlDocument               m_doc;
    std::stack<TiXmlElement *>  m_stack;
};