#pragma once

#include "common.h"

struct Student
{
    std::string sName;
    int age;
    float score;
    std::time_t t1;

    /*template <typename Archive>
    void Serialize(Archive &ar)
    {
    ar.Serialize("sName", sName);
    ar.Serialize("age", age);
    ar.Serialize("score", score);
    t1 = std::time(nullptr);
    ar.Serialize("t1", t1);
    }*/

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
};




namespace Serialization
{
    template <typename Archive>
    void Serialize(Archive &ar, Student &stu)
    {
        ar.Serialize("sName", stu.sName);
        ar.Serialize("age", stu.age);
        ar.Serialize("score", stu.score);
        stu.t1 = std::time(nullptr);
        ar.Serialize("t1", stu.t1);
    }
}

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

    /*template <typename Archive>
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
    }*/

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



namespace Serialization
{
    template <typename Archive>
    void Serialize(Archive &ar, Foo &foo)
    {
        ar.Serialize("vec", foo.vec);
        ar.Serialize("ls", foo.ls);
        ar.Serialize("st", foo.st);
        ar.Serialize("deq", foo.deq);
        ar.Serialize("mp", foo.mp);
        ar.Serialize("set", foo.set);
        ar.Serialize("un_mp", foo.un_mp);
        ar.Serialize("un_set", foo.un_set);
    }
}