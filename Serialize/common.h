#pragma once

#ifdef  WIN32
#pragma warning(disable:4146)
#endif //  WIN32

#include <cstdint>
#include <ctime>

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

#include "RapidxmlWrap.h"
#include "RapidjsonWrap.h"
#include "YamlWrap.h"
#include "any.h"

namespace Serialization
{
    struct Node
    {   
        Any                 elem;
        std::vector<Any>    children;

        Node()
        {
            children.clear();
        }
    };
}