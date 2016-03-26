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

template <bool T1, bool T2>
struct intrusive_if
{

};

template<>
struct intrusive_if<false, true>
{
    const static bool no = false;
    const static bool yes_class = true;
};

template<>
struct intrusive_if<true, true>
{	// type is _Ty for _Test
    const static bool yes = true;
    const static bool yes_class = true;
};
