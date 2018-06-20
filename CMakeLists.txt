cmake_minimum_required(VERSION 3.9.0)
project(exa VERSION 0.1)

option(EXA_BUILD_TESTS "Build test cases." ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(exa)

if(EXA_BUILD_TESTS)
    add_subdirectory(exa_test)
endif()
