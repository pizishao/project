#pragma once

#include "SerializeHeader.h"
#include "SerializeTraits.h"

#include "TinyXML/tinyxml.h"

#ifdef  WIN32
#pragma warning(disable:4146)
#endif //  WIN32

class XmlOutPutArchive
{
public:    
    XmlOutPutArchive(SerialEncodeType type)
    {
        TiXmlDeclaration xmlDeclaration("1.0", type == SerialEncodeType::GB2312 ? "gb2312" : "utf-8", "");
        m_doc.InsertEndChild(xmlDeclaration);

        TiXmlElement rootElem("Object");
        TiXmlNode *rootNode = m_doc.InsertEndChild(rootElem);

        m_stack.push(rootNode->ToElement());
    }

    ~XmlOutPutArchive(){}

private:
    TiXmlElement* WriteValue(const char *tag, std::string value)
    {
        if (m_stack.empty())
        {
            assert(false);
            return nullptr;
        }

        TiXmlElement elem(tag);
        TiXmlText text(value.c_str());

        elem.InsertEndChild(text);
        TiXmlElement *parent = m_stack.top();
        TiXmlElement *pElem = (TiXmlElement *)parent->InsertEndChild(elem);

        return pElem;
    }

    void StartObject(const char *tag)
    {
        if (m_stack.empty())
        {
            assert(false);
            return;
        }

        TiXmlElement elem(tag);
        TiXmlElement *parent = m_stack.top();

        TiXmlElement *pElem = (TiXmlElement *)parent->InsertEndChild(elem);
        m_stack.push(pElem);
    }

    void EndObject(const char *tag)
    {
        if (m_stack.empty())
        {
            assert(false);
            return;
        }

        TiXmlElement *pElem = m_stack.top();
        if (strcmp(pElem->Value(), tag) != 0)
        {
            assert(false);
            return;
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

    template <typename T, int N>
    void Serialize(const char *tag, T(&array)[N])
    {
        StartArray(tag);

        for (int i = 0; i < N; i++)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, array[i]);
        }

        EndArray(tag);
    }

    template <typename T, int N1, int N2>
    void Serialize(const char *tag, T(&array)[N1][N2])
    {
        StartArray(tag);

        for (int i = 0; i < N1; i++)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, array[i]);
        }

        EndArray(tag);
    }

    template <typename T, int N1, int N2, int N3>
    void Serialize(const char *tag, T(&array)[N1][N2][N3])
    {
        StartArray(tag);

        for (int i = 0; i < N1; i++)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i);
            Serialize(buffer, array[i]);
        }

        EndArray(tag);
    }

    template <typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
        Serialize(const char *tag, T &obj)
    {
        StartObject(tag);
        obj.Serialize(*this);
        EndObject(tag);
    }

    template <typename T>
    void Serialize(const char *tag, std::vector<T> &vec)
    {
        StartArray(tag);

        int i = 0;
        for (auto &pos : vec)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i++);
            Serialize(buffer, pos);
        }

        EndArray(tag);
    }

    template <typename T>
    void Serialize(const char *tag, std::list<T> &ls)
    {
        StartArray(tag);

        int i = 0;
        for (auto &pos : ls)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i++);
            Serialize(buffer, pos);
        }

        EndArray(tag);
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

        int i = 0;
        while (!tmpStack.empty())
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i++);
            Serialize(buffer, tmpStack.top());
            st.emplace(tmpStack.top());
            tmpStack.pop();
        }

        EndArray(tag);
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

        int i = 0;
        while (!tmpDeq.empty())
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i++);
            Serialize(buffer, tmpDeq.front());
            deq.emplace_back(tmpDeq.front());
            tmpDeq.pop_front();
        }

        EndArray(tag);
    }

    template <typename _Kty, typename _Ty>
    void Serialize(const char *tag, std::map<_Kty, _Ty> &mp)
    {
        StartArray(tag);

        int i = 0;
        for (auto &pos : mp)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i++);
            StartObject(buffer);

            Serialize("key", pos.first);
            Serialize("value", pos.second);

            EndObject(buffer);
        }

        EndArray(tag);
    }

    template <typename T>
    void Serialize(const char *tag, std::set<T> &set)
    {
        StartArray(tag);

        int i = 0;
        for (auto &pos : set)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i++);
            Serialize(buffer, (T &)pos);
        }

        EndArray(tag);
    }

    template <typename _Kty, typename _Ty>
    void Serialize(const char *tag, std::unordered_map<_Kty, _Ty> &mp)
    {
        StartArray(tag);

        int i = 0;
        for (auto &pos : mp)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i++);
            StartObject(buffer);

            Serialize("key", pos.first);
            Serialize("value", pos.second);

            EndObject(buffer);
        }

        EndArray(tag);
    }

    template <typename T>
    void Serialize(const char *tag, std::unordered_set<T> &set)
    {
        StartArray(tag);

        int i = 0;
        for (auto &pos : set)
        {
            char buffer[50] = { 0 };

            sprintf(buffer, "item%d", i++);
            Serialize(buffer, (T &)pos);
        }

        EndArray(tag);
    }

    template <typename T>
    void operator << (T &obj)
    {
        obj.Serialize(*this);
    }

    std::string GetXmlText()
    {
        TiXmlPrinter printer;
        m_doc.Accept(&printer);

        assert(!m_stack.empty() && m_stack.top() == m_doc.RootElement());

        return printer.CStr();
    }

private:
    TiXmlDocument               m_doc;
    std::stack<TiXmlElement *>  m_stack;
};