#pragma once

#include "rapidxml/rapidxml_utils.hpp"  
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml.hpp"  

#include "common.h"

typedef rapidxml::xml_node<>*       XmlNodePtr;
typedef rapidxml::xml_attribute<>*  XmlAttributePtr;

namespace Serialization
{
    typedef Node<XmlNodePtr>            XmlSeriNode;
}