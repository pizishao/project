﻿// Serializtion.cpp : Defines the entry point for the console application.
//

#pragma execution_character_set("utf-8")

#include "stdafx.h"

#include <stdio.h>
#include <tuple>
#include <ctime>

#include "JSONInputArchive.h"
#include "JSONOutputArchive.h"
#include "XMLOutputArchive.h"
#include "XMLInputArchive.h"

struct Student
{
    std::string sName;
    int age;
    float score;
    std::time_t t1;

    bool operator==(const Student &rhs) const 
    {
        if (sName == rhs.sName && age == rhs.age && score == rhs.score)
        {
            return true;
        }

        return false;
    }

    bool operator<(const Student &rhs) const
    {
        if (sName < rhs.sName)
        {
            return true;
        }
        else
        {
            if (sName == rhs.sName)
            {
                if (age < rhs.age)
                {
                    return true;
                }
                else
                {
                    if (age == rhs.age)
                    {
                        if (score < rhs.score)
                        {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    template <typename Archive>
    void Serialize(Archive &ar)
    {
        ar.Serialize("sName", sName);
        ar.Serialize("age", age);
        ar.Serialize("score", score);
        t1 = std::time(nullptr);
        ar.Serialize("t1", t1);
    }
};

namespace std
{
    template<> struct hash < Student >
    {
        typedef Student argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            result_type const h1(std::hash<std::string>()(s.sName));
            result_type const h2(std::hash<int>()(s.age));
            result_type const h3(std::hash<std::int64_t>()(s.score));
            return h1 ^ (h2 << 1); // or use boost::hash_combine
        }
    };
}

struct Foo
{
    std::vector<Student> vec;
    std::list<Student> ls;
    std::stack<Student> st;
    std::deque<Student> deq;
    std::map<std::string, Student> mp;
    std::set<Student> set;
    std::unordered_map<std::string, Student> un_mp;
    std::unordered_set<Student> un_set;

    template <typename Archive>
    void Serialize(Archive &ar)
    {
        ar.Serialize("vec", vec);
        ar.Serialize("ls", ls);
        ar.Serialize("st", st);
        ar.Serialize("deq", deq);
        ar.Serialize("mp", mp);
        ar.Serialize("set", set);
        ar.Serialize("un_mp", un_mp);
        ar.Serialize("un_set", un_set);
    }

    void Clear()
    {
        vec.clear();
        ls.clear();
        std::stack<Student> tmpStack;
        st.swap(tmpStack);
        std::deque<Student> tmpDeq;
        deq.swap(tmpDeq);
        mp.clear();
        set.clear();
        un_mp.clear();
        un_set.clear();
    }
};

int _tmain(int argc, _TCHAR* argv[])
{
    Foo foo;

    Student st1;
    Student st2;
    Student st3;

    st1.sName = "辉辉";
    st1.age = 25;
    st1.score = 24;

    st2.sName = "老K";
    st2.age = 26;
    st2.score = 25;

    st3.sName = "将军";
    st3.age = 27;
    st3.score = 26;

    foo.vec.push_back(st1);
    foo.vec.push_back(st2);
    foo.vec.push_back(st3);

    foo.ls.push_back(st1);
    foo.ls.push_back(st2);
    foo.ls.push_back(st3);

    foo.st.emplace(st1);
    foo.st.emplace(st2);
    foo.st.emplace(st3);

    foo.deq.push_back(st1);
    foo.deq.push_back(st2);
    foo.deq.push_back(st3);

    foo.mp.insert({st1.sName, st1});
    foo.mp.insert({ st2.sName, st2 });
    foo.mp.insert({ st3.sName, st3 });


    foo.un_mp.insert({ st1.sName, st1 });
    foo.un_mp.insert({ st2.sName, st2 });
    foo.un_mp.insert({ st3.sName, st3 });

    foo.set.insert(st1);
    foo.set.insert(st2);
    foo.set.insert(st3);


    foo.un_set.insert(st1);
    foo.un_set.insert(st2);
    foo.un_set.insert(st3);

    //JsonOutPutArchive oAchive;
    XmlOutPutArchive oAchive;    
    oAchive << foo;

    std::string s = oAchive.GetXmlText();
    //std::string s = oAchive.GetJsonText();
    foo.Clear();

    //JsonInPutArchive iAchive;
    XmlInPutArchive iAchive;
    iAchive.Load(s);
    iAchive >> foo;

     getchar();

	return 0;
}
