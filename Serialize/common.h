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

namespace Serialization
{
    template<typename T>
    struct Node
    {
        T                 elem;
        std::vector<T>    children;

        Node()
        {
            elem = T();
            children.clear();
        }
    };
}