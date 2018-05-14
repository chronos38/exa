#pragma once

#include <mutex>
#include <type_traits>
#include <functional>
#include <chrono>

namespace exa
{
    template <class Mutex = std::mutex>
    class lockable
    {
    public:
        void lock()
        {
            mutex_.lock();
        }

        void unlock()
        {
            mutex_.unlock();
        }

        bool try_lock()
        {
            return mutex_.try_lock();
        }

    private:
        mutable Mutex mutex_;
    };

    struct lock
    {
        template <class BasicLockable, class Callable, class = std::enable_if<std::is_invocable_v<Callable>>>
        constexpr lock(BasicLockable&& l, Callable&& f)
        {
            std::lock_guard<BasicLockable> _(l);
            std::invoke(f);
        }

        template <class Lockable1, class Lockable2, class Callable, class = std::enable_if<std::is_invocable_v<Callable>>>
        constexpr lock(Lockable1&& l1, Lockable2&& l2, Callable&& f)
        {
            std::scoped_lock<Lockable1, Lockable2> _(l1, l2);
            std::invoke(f);
        }

        template <class Lockable1, class Lockable2, class Lockable3, class Callable,
                  class = std::enable_if<std::is_invocable_v<Callable>>>
        constexpr lock(Lockable1&& l1, Lockable2&& l2, Lockable3&& l3, Callable&& f)
        {
            std::scoped_lock<Lockable1, Lockable2, Lockable3> _(l1, l2, l3);
            std::invoke(f);
        }

        template <class Lockable1, class Lockable2, class Lockable3, class Lockable4, class Callable,
                  class = std::enable_if<std::is_invocable_v<Callable>>>
        constexpr lock(Lockable1&& l1, Lockable2&& l2, Lockable3&& l3, Lockable4&& l4, Callable&& f)
        {
            std::scoped_lock<Lockable1, Lockable2, Lockable3, Lockable4> _(l1, l2, l3, l4);
            std::invoke(f);
        }

        template <class Lockable1, class Lockable2, class Lockable3, class Lockable4, class Lockable5, class Callable,
                  class = std::enable_if<std::is_invocable_v<Callable>>>
        constexpr lock(Lockable1&& l1, Lockable2&& l2, Lockable3&& l3, Lockable4&& l4, Lockable5&& l5, Callable&& f)
        {
            std::scoped_lock<Lockable1, Lockable2, Lockable3, Lockable4, Lockable5> _(l1, l2, l3, l4, l5);
            std::invoke(f);
        }
    };
}
