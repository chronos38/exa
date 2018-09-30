#pragma once

#include <exa/task.hpp>
#include <exa/enum_flag.hpp>

#include <gsl/gsl>

#include <functional>
#include <cstddef>
#include <variant>
#include <future>

namespace exa
{
    namespace detail
    {
        enum class io_state
        {
            none = 0,
            read = 0x1,
            write = 0x2,
            abort = 0x4,
            read_write = read | write
        };

        using write_handle = std::function<size_t(gsl::span<const uint8_t>)>;
        using read_handle = std::function<size_t(gsl::span<uint8_t>)>;
        using state_handle = std::function<io_state()>;

        struct write_command
        {
            write_handle handle;
            gsl::span<const uint8_t> buffer;
        };

        struct read_command
        {
            read_handle handle;
            gsl::span<uint8_t> buffer;
        };

        using command_variant = std::variant<write_command, read_command>;

        struct io_task_data
        {
            command_variant data;
            state_handle state;
        };

        class io_task
        {
        public:
            static std::future<size_t> run(const io_task_data& data);

        private:
            struct async_result
            {
                bool done = false;
                size_t bytes = 0;
            };

            static size_t HandleReadWrite(const io_task_data& data);
            static void run_async(std::function<bool()> cb);
        };
    }
}
