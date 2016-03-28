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
//#include "XMLInputArchive.h"
#include "YAMLOutputArchive.h"
//#include "YAMLInputArchive.h"

#include "OutputArchive.h"
#include "InputArchive.h"

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

    //Serialization::OutputArchive<Serialization::YamlOutputArchive, false> outputArchiveYaml;
    Serialization::OutputArchive<Serialization::JsonOutPutArchive, false> outputArchiveJson;
    //Serialization::OutputArchive<Serialization::XmlOutPutArchive, false> outputArchiveXml;

    //outputArchiveYaml << foo;
    outputArchiveJson << foo;
    //outputArchiveXml << foo;
    
    //std::string sYaml = outputArchiveYaml.c_str();
    std::string sJson = outputArchiveJson.c_str();
    //std::string sXml = outputArchiveXml.c_str();
    foo.Clear();

    Serialization::InputArchive<Serialization::JsonInPutArchive, false>    iArchiveJson;
    //Serialization::InputArchive<Serialization::YamlInputArchive, false>    iArchiveYaml;
    //Serialization::InputArchive<Serialization::XmlInPutArchive, false>     iArchiveXml;
    iArchiveJson.Load(sJson);
    iArchiveJson >> foo;
    /*foo.Clear();
    iArchiveYaml.Load(sYaml);
    iArchiveYaml >> foo;

    foo.Clear();
    iArchiveXml.Load(sXml);
    iArchiveXml >> foo;*/

     getchar();

	return 0;
}

