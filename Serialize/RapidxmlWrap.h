#pragma once

#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"  
#include "rapidxml/rapidxml_print.hpp"

enum SerialEncodeType
{
    GB2312,
    UTF8,
};

typedef rapidxml::xml_node<>*       XmlNodePtr;
typedef rapidxml::xml_attribute<>*  XmlAttributePtr;