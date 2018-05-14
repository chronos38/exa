#pragma once

#include <functional>
#include <type_traits>

namespace exa
{
    struct scope
    {
        template <class T, class Callable, class = std::enable_if<std::is_invocable_v<Callable, T>>>
        constexpr scope(T&& t, Callable&& f)
        {
            std::invoke(f, t);
        }

        template <class T1, class T2, class Callable, class = std::enable_if<std::is_invocable_v<Callable, T1, T2>>>
        constexpr scope(T1&& t1, T2&& t2, Callable&& f)
        {
            std::invoke(f, t1, t2);
        }

        template <class T1, class T2, class T3, class Callable,
                  class = std::enable_if<std::is_invocable_v<Callable, T1, T2, T3>>>
        constexpr scope(T1&& t1, T2&& t2, T3&& t3, Callable&& f)
        {
            std::invoke(f, t1, t2, t3);
        }

        template <class T1, class T2, class T3, class T4, class Callable,
                  class = std::enable_if<std::is_invocable_v<Callable, T1, T2, T3, T4>>>
        constexpr scope(T1&& t1, T2&& t2, T3&& t3, T4&& t4, Callable&& f)
        {
            std::invoke(f, t1, t2, t3, t4);
        }

        template <class T1, class T2, class T3, class T4, class T5, class Callable,
                  class = std::enable_if<std::is_invocable_v<Callable, T1, T2, T3, T4, T5>>>
        constexpr scope(T1&& t1, T2&& t2, T3&& t3, T4&& t4, T5&& t5, Callable&& f)
        {
            std::invoke(f, t1, t2, t3, t4, t5);
        }
    };
}
