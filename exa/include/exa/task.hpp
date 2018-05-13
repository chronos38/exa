#pragma once

#include <exa/lockable.hpp>

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
    class task
    {
    public:
        template <class Function, class = std::enable_if_t<std::is_void_v<std::invoke_result_t<Function>>>>
        static std::future<void> run(Function&& f)
        {
            auto p = std::make_shared<std::promise<void>>();
            {
                std::unique_lock<lockable<>> lock(instance.task_queue_);
                instance.task_queue_.push_back([f, p] {
                    try
                    {
                        f();
                        p->set_value();
                    }
                    catch (...)
                    {
                        p->set_exception(std::current_exception());
                    }
                });
            }
            instance.task_signal_.notify_one();
            return p->get_future();
        }

        template <class Function, class = std::enable_if_t<!std::is_void_v<std::invoke_result_t<Function>>>>
        static std::future<std::invoke_result_t<Function>> run(Function&& f)
        {
            using return_type = std::invoke_result_t<Function>;
            auto p = std::make_shared<std::promise<return_type>>();
            {
                std::unique_lock<lockable<>> lock(instance.task_queue_);
                instance.task_queue_.push_back([f, p] {
                    try
                    {
                        auto&& r = f();
                        p->set_value(r);
                    }
                    catch (...)
                    {
                        p->set_exception(std::current_exception());
                    }
                });
            }
            instance.task_signal_.notify_one();
            return p->get_future();
        }

        static void initialize(std::size_t thread_count);

        static void deinitialize();

    private:
        ~task();
        void shutdown();

        static void work();

        struct task_queue : public std::deque<std::function<void()>>, public lockable<>
        {
        };

        static task instance;
        std::atomic_bool run_;
        std::condition_variable_any task_signal_;
        std::vector<std::thread> threads_;
        task_queue task_queue_;
    };
}
