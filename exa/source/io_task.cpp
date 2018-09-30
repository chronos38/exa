#include <exa/detail/io_task.hpp>

#include <cstdint>

namespace exa
{
    namespace detail
    {
        std::future<std::any> io_task::run(const io_task_data& data)
        {
            auto operation = [](const io_task_data& data) {
                switch (data.state())
                {
                    case io_state::abort:
                        return async_result{true, std::any()};
                    case io_state::ready:
                        return async_result{true, data.callback(data.argument)};
                    default:
                        return async_result{false, std::any()};
                }
            };

            auto promise = std::make_shared<std::promise<std::any>>();

            run_async([data, operation, promise] {
                try
                {
                    auto r = operation(data);

                    if (r.done)
                    {
                        promise->set_value(r.result);
                    }

                    return r.done;
                }
                catch (...)
                {
                    promise->set_exception(std::current_exception());
                    return true;
                }
            });

            return promise->get_future();
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
