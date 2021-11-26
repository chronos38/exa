cmake_minimum_required(VERSION 3.12)
project(exa VERSION 0.1)

option(EXA_BUILD_TESTS "Build test cases." ON)
option(EXA_MSVC_UNICODE "Use Unicode strings for MSVC instead of ANSI." ON)
option(EXA_MSVC_STATIC_RUNTIME "Use static MSVC runtime instead of DLLs." OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(/D_CRT_SECURE_NO_WARNINGS /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /D_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS /MP)

    if(EXA_MSVC_STATIC_RUNTIME)
        add_compile_options($<CONFIG:Debug>:/MTd $<CONFIG:RelWithDebInfo>:/MT $<CONFIG:MinSizeRel>:/MT $<CONFIG:Release>:/MT)
    else()
        add_compile_options($<CONFIG:Debug>:/MDd $<CONFIG:RelWithDebInfo>:/MD $<CONFIG:MinSizeRel>:/MD $<CONFIG:Release>:/MD)
    endif()

    if(EXA_MSVC_UNICODE)
        add_compile_options(/Zc:wchar_t /DUNICODE /D_UNICODE)
    endif()
endif()

add_subdirectory(exa)

if(EXA_BUILD_TESTS)
    add_subdirectory(exa_test)
endif()
