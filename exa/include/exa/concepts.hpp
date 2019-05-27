#pragma once

#include <functional>
#include <type_traits>
#include <mutex>

namespace exa
{
    template <class Mutex>
    class lockable
    {
    public:
        virtual ~lockable()
        {
            if (mutex_)
            {
                delete mutex_;
                mutex_ = nullptr;
            }
        }

        constexpr void lock() const
        {
            mutex_->lock();
        }

        constexpr void unlock() const
        {
            mutex_->unlock();
        }

        constexpr bool try_lock() const
        {
            return mutex_->try_lock();
        }

        constexpr operator Mutex() const
        {
            return *mutex_;
        }

        constexpr Mutex& mutex() const
        {
            return mutex_;
        }

    private:
        Mutex* mutex_ = new Mutex();
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

    struct scope
    {
        template <class Callable, class = std::enable_if<std::is_invocable_v<Callable>>>
        constexpr scope(Callable&& f)
        {
            std::invoke(f);
        }

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
