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

template <typename T>
class has_member_serialize_func
{
    template<typename U, void (U::*)()> struct HELPS;
    template<typename U> static char Test(HELPS<U, &U::Serialize>*);
    template<typename U> static int Test(...);
    const static bool Has = sizeof(Test<T>(0)) == sizeof(char);

public:
    static const bool value = (sizeof(Test<T>(0)) == sizeof(char));
};

/*
template <typename T, typename archive>
class not_has_member_serialize_func
{
    typedef char one;
    typedef long two;

    template <typename C, typename ar> static one test(decltype(&C::Serialize<ar>));
    template <typename C, typename ar> static two test(...);

public:
    static const bool value = (sizeof(test<T, archive>(0)) != sizeof(char) && std::is_class<T>::value
        && !std::is_same<std::string, T>::value);
};*/