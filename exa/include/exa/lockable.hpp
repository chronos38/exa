#pragma once

#include <mutex>

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
        Mutex mutex_;
    };
}
