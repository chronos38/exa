#pragma once

#include <exa/task.hpp>
#include <exa/enum_flag.hpp>

#include <gsl/gsl>

#include <functional>
#include <cstddef>
#include <future>
#include <any>

namespace exa
{
    namespace detail
    {
        enum class io_state
        {
            not_ready,
            ready,
            abort
        };

        using callback_handle = std::function<std::any(std::any)>;
        using state_handle = std::function<io_state()>;

        struct io_task_data
        {
            callback_handle callback;
            std::any argument;
            state_handle state;
        };

        class io_task
        {
        public:
            static std::future<std::any> run(const io_task_data& data);

        private:
            struct async_result
            {
                bool done = false;
                std::any result;
            };

            static void run_async(std::function<bool()> cb);
        };
    }
}
