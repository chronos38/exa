#include <exa/detail/io_task.hpp>

#include <functional>

namespace exa
{
    namespace detail
    {
        void io_task::run_internal(std::function<bool()> callback)
        {
            if (!callback())
            {
                task::push(std::bind(&io_task::run_internal, callback));
            }
        }
    }
}
