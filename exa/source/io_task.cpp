#include <exa/detail/io_task.hpp>

#include <cstdint>

namespace exa
{
    namespace detail
    {
        std::future<size_t> io_task::run(const io_task_data& data)
        {
            auto operation = [](const io_task_data& data) {
                switch (data.state())
                {
                    case io_state::abort:
                        return async_result{true, 0};
                    case io_state::read:
                    case io_state::write:
                    case io_state::read_write:
                        return async_result{true, HandleReadWrite(data)};
                    default:
                        return async_result{false, 0};
                }
            };

            auto promise = std::make_shared<std::promise<size_t>>();

            run_async([data, operation, promise] {
                try
                {
                    auto r = operation(data);

                    if (r.done)
                    {
                        promise->set_value(r.bytes);
                        return true;
                    }

                    return false;
                }
                catch (...)
                {
                    promise->set_exception(std::current_exception());
                    return true;
                }
            });

            return promise->get_future();
        }

        size_t io_task::HandleReadWrite(const io_task_data& io)
        {
            return std::visit(
                [&](auto&& data) {
                    using T = std::decay_t<decltype(data)>;

                    if constexpr (std::is_same_v<T, read_command>)
                    {
                        return data.handle(data.buffer);
                    }
                    else if constexpr (std::is_same_v<T, write_command>)
                    {
                        return data.handle(data.buffer);
                    }
                    else
                    {
                        return 0;
                    }
                },
                io.data);
        }

        void io_task::run_async(std::function<bool()> cb)
        {
            if (!cb())
            {
                task::run(std::bind(&io_task::run_async, cb));
            }
        }
    }
}
