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
            auto p = std::make_shared<std::promise<void>>();
            lock(instance.task_queue_, [&] {
                instance.task_queue_.push_back([f, p] {
                    try
                    {
                        std::invoke(f);
                        p->set_value();
                    }
                    catch (...)
                    {
                        p->set_exception(std::current_exception());
                    }
                });
            });
            instance.task_signal_.notify_one();
            return p->get_future();
        }

        template <class Function, class = std::enable_if_t<!std::is_void_v<std::invoke_result_t<Function>>>>
        static std::future<std::invoke_result_t<Function>> run(Function&& f)
        {
            static_assert(std::is_invocable_v<Function>);
            using return_type = std::invoke_result_t<Function>;
            auto p = std::make_shared<std::promise<return_type>>();
            lock(instance.task_queue_, [&] {
                instance.task_queue_.push_back([f, p] {
                    try
                    {
                        auto&& r = std::invoke(f);
                        p->set_value(r);
                    }
                    catch (...)
                    {
                        p->set_exception(std::current_exception());
                    }
                });
            });
            instance.task_signal_.notify_one();
            return p->get_future();
        }

        static void initialize(std::size_t thread_count);

        static void deinitialize(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(100));

        static size_t available_tasks();

        static size_t total_tasks();

    private:
        using task_callback = std::function<void()>;

        struct task_queue : public std::deque<task_callback>, public lockable<std::mutex>
        {
        };

        task();
        ~task();
        void shutdown(const std::chrono::milliseconds& timeout);
        void work();

        template <class TaskCallback>
        static void push(TaskCallback&& cb)
        {
            lock(instance.task_queue_, [&] { instance.task_queue_.push_back(cb); });
        }

        static task instance;
        std::atomic_bool run_;
        std::atomic_int waiting_;
        std::atomic_int exited_;
        std::condition_variable_any task_signal_;
        std::vector<std::thread> threads_;
        task_queue task_queue_;

        friend class detail::io_task;
    };
}
