#include <exa/task.hpp>
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

        instance.run_ = true;

        for (size_t i = 0; i < thread_count; ++i)
        {
            instance.threads_.push_back(std::thread(std::bind(&work)));
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
        {
            std::unique_lock<lockable<>> lock(task_queue_);
            task_queue_.clear();
            task_signal_.notify_all();
        }

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
        while (instance.run_)
        {
            std::function<void()> f;
            {
                std::unique_lock<lockable<>> lock(instance.task_queue_);
                instance.task_signal_.wait(lock);

                if (!instance.run_)
                {
                    break;
                }
                else if (instance.task_queue_.empty())
                {
                    continue;
                }

                f = instance.task_queue_.front();
                instance.task_queue_.pop_front();
            }

            f();
        }
    }
}
