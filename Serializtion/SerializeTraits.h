#pragma once

#include <type_traits>

template<typename T>
struct is_signedBigInt
{
    static const bool value = (sizeof(T) > sizeof(int) && std::is_signed<T>::value && std::is_integral<T>::value);
};

template<typename T>
struct is_unsignedBigInt
{
    static const bool value = (sizeof(T) > sizeof(int) && std::is_unsigned<T>::value && std::is_integral<T>::value);
};

template<typename T>
struct is_signedSmallInt
{
    static const bool value = (sizeof(T) <= sizeof(int) && std::is_signed<T>::value && std::is_integral<T>::value);
};

template<typename T>
struct is_unsignedSmallInt
{
    static const bool value = (sizeof(T) <= sizeof(int) && std::is_unsigned<T>::value && std::is_integral<T>::value);
};
