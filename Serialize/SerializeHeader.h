#pragma once

#ifdef  WIN32
#pragma warning(disable:4146)
#endif //  WIN32

#include <cstdint>
#include <ctime>

#include <type_traits>
#include <assert.h>

#include <string>

#include <vector>
#include <list>
#include <stack>
#include <deque>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <algorithm>

#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"  
#include "rapidxml/rapidxml_print.hpp"

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

enum SerialEncodeType
{
    GB2312,
    UTF8,
};

typedef rapidxml::xml_node<>*       XmlNodePtr;
typedef rapidxml::xml_attribute<>*  XmlAttributePtr;