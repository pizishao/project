// Serializtion.cpp : Defines the entry point for the console application.
//

//#pragma execution_character_set("utf-8")

#include <stdio.h>
#include <tuple>
#include <ctime>
#include <string>

#include "pre.h"

#include "JSONInputArchive.h"
#include "JSONOutputArchive.h"
#include "XMLOutputArchive.h"
#include "XMLInputArchive.h"
#include "YAMLOutputArchive.h"
#include "YAMLInputArchive.h"

#include "OutputArchive.h"
#include "InputArchive.h"

/*
std::string GBToUTF8(const char* str)
{
    std::string result;
    WCHAR *strSrc;
    char *szRes;

    int i = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
    strSrc = new WCHAR[i + 1];
    MultiByteToWideChar(CP_ACP, 0, str, -1, strSrc, i);

    i = WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, nullptr, 0, nullptr, nullptr);
    szRes = new char[i + 1];
    int j = WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, szRes, i, nullptr, nullptr);

    result = szRes;
    delete[]strSrc;
    delete[]szRes;

    return result;
}*/

int main(int argc, char* argv[])
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

    st3.sName = "叶枫";
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
    //XmlOutPutArchive oAchive(SerialEncodeType::UTF8);
    Serialization::OutputArchive<Serialization::YamlOutputArchive, false> outputArchiveYaml;
    Serialization::OutputArchive<Serialization::JsonOutPutArchive, false> outputArchiveJson;
    Serialization::OutputArchive<Serialization::XmlOutPutArchive, false> outputArchiveXml;

    outputArchiveYaml << foo;
    outputArchiveJson << foo;
    outputArchiveXml << foo;

    /*decltype(intrusive_if<true, std::is_class<Student>::value>::yes_class, intrusive_if<true, std::is_class<Student>::value>::yes, void())*/
    

    //int a = has_member_serialize_func<std::vector<Student>, Serialization::YamlOutputArchive>::value;

    //auto p = &Foo::Serialize<Serialization::YamlOutputArchive>;
    //std::string name = typeid(decltype(p)).name();

    //int value = std::is_member_function_pointer<void(__thiscall Foo::*)(class Serialization::YamlOutputArchive &)>::value;
    //int value = std::is_member_function_pointer<decltype(&Foo::Serialize<Serialization::YamlOutputArchive>)>::value;


    //std::string s = oAchive.GetXmlText();
    //std::string s = oAchive.GetJsonText();
    
    std::string sYaml = outputArchiveYaml.c_str();
    std::string sJson = outputArchiveJson.c_str();
    std::string sXml = outputArchiveXml.c_str();
    foo.Clear();

    //Serialization::YamlInputArchive iAchive;
    //iAchive.Load(s);

    Serialization::InputArchive<Serialization::JsonInPutArchive, false>    iArchiveJson;
    Serialization::InputArchive<Serialization::YamlInputArchive, false>    iArchiveYaml;
    Serialization::InputArchive<Serialization::XmlInPutArchive, false>     iArchiveXml;
    iArchiveJson.Load(sJson);
    iArchiveJson >> foo;
    foo.Clear();
    iArchiveYaml.Load(sYaml);
    iArchiveYaml >> foo;

    foo.Clear();
    iArchiveXml.Load(sXml);
    iArchiveXml >> foo;

     getchar();

	return 0;
}

