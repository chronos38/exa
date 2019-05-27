#pragma once

#include <exa/concepts.hpp>

#include <future>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <atomic>
#include <vector>
#include <type_traits>
#include <chrono>

namespace exa
{
    namespace detail
    {
        class io_task;
    }

    class task
    {
    public:
        template <class Function, class = std::enable_if_t<std::is_void_v<std::invoke_result_t<Function>>>>
        static std::future<void> run(Function&& f)
        {
            static_assert(std::is_invocable_v<Function>);
            // auto p = std::make_unique<std::promise<void>>();
            // auto r = p->get_future();
            // taskflow_.emplace([f, p = std::move(p)] {
            //    try
            //    {
            //        std::invoke(f);
            //        p->set_value();
            //    }
            //    catch (...)
            //    {
            //        p->set_exception(std::current_exception());
            //    }
            //});
            // return r;
            // TODO: Create and execute task.
            return std::async(std::launch::async, f);
        }

        template <class Function, class = std::enable_if_t<!std::is_void_v<std::invoke_result_t<Function>>>>
        static std::future<std::invoke_result_t<Function>> run(Function&& f)
        {
            static_assert(std::is_invocable_v<Function>);
            // using return_type = std::invoke_result_t<Function>;
            // auto p = std::make_unique<std::promise<return_type>>();
            // auto r = p->get_future();
            // taskflow_.emplace([f, p = std::move(p)] {
            //    try
            //    {
            //        auto&& r = std::invoke(f);
            //        p->set_value(std::move(r));
            //    }
            //    catch (...)
            //    {
            //        p->set_exception(std::current_exception());
            //    }
            //});
            // return r;
            return std::async(std::launch::async, f);
        }
    };
}
