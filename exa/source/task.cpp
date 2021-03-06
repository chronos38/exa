#include <exa/task.hpp>
#include <exa/concepts.hpp>

#include <functional>

using namespace std::chrono_literals;

namespace exa
{
    task task::instance;

    void task::initialize(std::size_t thread_count)
    {
        if (thread_count == 0)
        {
            throw std::out_of_range("Thread count must be greater than 0.");
        }
        if (instance.run_)
        {
            throw std::runtime_error("Tasks are already running. Call deinitialize first.");
        }

        instance.run_ = true;

        for (size_t i = 0; i < thread_count; ++i)
        {
            instance.threads_.push_back(std::thread(std::bind(&task::work, &instance)));
        }
    }

    void task::deinitialize(const std::chrono::milliseconds& timeout)
    {
        instance.shutdown(timeout);
    }

    size_t task::available_tasks()
    {
        return static_cast<size_t>(instance.waiting_);
    }

    size_t task::total_tasks()
    {
        return instance.threads_.size();
    }

    task::task()
    {
        waiting_ = 0;
        exited_ = 0;
    }

    task::~task()
    {
        shutdown(0ms);
    }

    void task::shutdown(const std::chrono::milliseconds& timeout)
    {
        run_ = false;

        while ((waiting_ + exited_) != static_cast<int>(threads_.size()))
        {
            std::this_thread::sleep_for(timeout);
        }

        lock(task_queue_, [this] {
            task_queue_.clear();
            task_signal_.notify_all();
        });

        for (auto& t : threads_)
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        threads_.clear();
        waiting_ = 0;
        exited_ = 0;
    }

    void task::work()
    {
        while (run_)
        {
            std::function<void()> f;

            scope(std::unique_lock(task_queue_), [&](auto&& lock) {
                waiting_ += 1;
                task_signal_.wait(lock);
                waiting_ -= 1;

                if (!run_)
                {
                    return;
                }
                if (task_queue_.empty())
                {
                    return;
                }

                f = task_queue_.front();
                task_queue_.pop_front();
            });

            if (f)
            {
                std::invoke(f);
            }
        }

        exited_ += 1;
    }
}
