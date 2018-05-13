#include <pch.h>
#include <exa/task.hpp>

using namespace exa;

TEST(task_test, init_deinit_several_times_success)
{
    for (size_t i = 1; i <= 10; ++i)
    {
        ASSERT_NO_THROW(task::deinitialize());
        ASSERT_NO_THROW(task::initialize(i));
    }

    // back to default
    auto n = std::max<size_t>(std::thread::hardware_concurrency() * 4, 2);
    ASSERT_NO_THROW(task::deinitialize());
    ASSERT_NO_THROW(task::initialize(n));
}
