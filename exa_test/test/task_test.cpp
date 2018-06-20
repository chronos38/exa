#include <pch.h>
#include <exa/task.hpp>

using namespace exa;
using namespace std::chrono_literals;

TEST(task_test, invalid_argument_throws)
{
    ASSERT_THROW(task::initialize(0), std::out_of_range);
}

TEST(task_test, already_running_task_throws)
{
    ASSERT_THROW(task::initialize(1), std::runtime_error);
}

TEST(task_test, init_deinit_several_times_success)
{
    for (size_t i = 1; i <= 10; ++i)
    {
        ASSERT_NO_THROW(task::deinitialize(0ms));
        ASSERT_NO_THROW(task::initialize(i));
    }

    // back to default
    auto n = std::max<size_t>(std::thread::hardware_concurrency(), 2);
    ASSERT_NO_THROW(task::deinitialize(0ms));
    ASSERT_NO_THROW(task::initialize(n));
}
