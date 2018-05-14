#include <exa/task.hpp>
#include <exa/concepts.hpp>

#include <functional>

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

    void task::deinitialize()
    {
        instance.shutdown();
    }

    task::~task()
    {
        shutdown();
    }

    void task::shutdown()
    {
        run_ = false;

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
    }

    void task::work()
    {
        while (run_)
        {
            std::function<void()> f;

            scope(std::unique_lock(task_queue_), [&](auto&& lock) {
                task_signal_.wait(lock);

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
                f();
            }
        }
    }
}
