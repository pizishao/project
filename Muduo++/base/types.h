#pragma once

#include <vector>
#include <memory>
#include <thread>

typedef std::vector<char>               vector_char;
typedef std::shared_ptr<std::thread>    thread_ptr;

template<typename T>
using vector_ptr=std::vector<std::shared_ptr<T>>;