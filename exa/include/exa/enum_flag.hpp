#pragma once

#include <type_traits>

namespace exa
{
    template <class T, class = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T operator~(T a)
    {
        return static_cast<T>(~static_cast<std::underlying_type_t<T>>(a));
    }

    template <class T, class = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T operator|(T a, T b)
    {
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b));
    }

    template <class T, class = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T operator&(T a, T b)
    {
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b));
    }

    template <class T, class = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T operator^(T a, T b)
    {
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) ^ static_cast<std::underlying_type_t<T>>(b));
    }

    template <class T, class = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T& operator|=(T& a, T b)
    {
        return static_cast<T&>(reinterpret_cast<std::underlying_type_t<T>&>(a) |= static_cast<std::underlying_type_t<T>>(b));
    }

    template <class T, class = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T& operator&=(T& a, T b)
    {
        return static_cast<T&>(reinterpret_cast<std::underlying_type_t<T>&>(a) &= static_cast<std::underlying_type_t<T>>(b));
    }

    template <class T, class = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T& operator^=(T& a, T b)
    {
        return static_cast<T&>(reinterpret_cast<std::underlying_type_t<T>&>(a) ^= static_cast<std::underlying_type_t<T>>(b));
    }

    template <class T, class = std::enable_if_t<std::is_enum_v<T>>>
    constexpr bool has_flag(T a, T b)
    {
        return static_cast<std::underlying_type_t<T>>(a & b) != static_cast<std::underlying_type_t<T>>(0);
    }
}
