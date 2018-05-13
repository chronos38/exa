#include <pch.h>
#include <exa/task.hpp>

int main(int argc, char** argv)
{
    auto n = std::max<size_t>(std::thread::hardware_concurrency() * 4, 2);
    exa::task::initialize(n);
    testing::InitGoogleTest(&argc, argv);
#ifdef _DEBUG
    auto r = RUN_ALL_TESTS();
    std::cout << "Press any button to continue . . .";
    getchar();
    return r;
#else
    return RUN_ALL_TESTS();
#endif
}
