cmake_minimum_required(VERSION 3.9.0)
project(exa VERSION 0.1)

option(EXA_BUILD_TESTS "Build test cases." ON)
option(EXA_USE_VCPKG "Use VCPKG instead of local build system." ON)
option(EXA_MSVC_UNICODE "Use unicode strings for MSVC instead of ansi." ON)
option(EXA_MSVC_STATIC_RUNTIME "Use static MSVC runtime instead of DLLs." OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    if(EXA_MSVC_STATIC_RUNTIME)
        set(EXA_MSVC_RUNTIME_FLAG_DEBUG "/MTd")
        set(EXA_MSVC_RUNTIME_FLAG_RELEASE "/MT")
    else()
        set(EXA_MSVC_RUNTIME_FLAG_DEBUG "/MDd")
        set(EXA_MSVC_RUNTIME_FLAG_RELEASE "/MD")
    endif()

    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS /W3 /GR /EHsc /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "${EXA_MSVC_RUNTIME_FLAG_DEBUG} /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "${EXA_MSVC_RUNTIME_FLAG_RELEASE} /O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${EXA_MSVC_RUNTIME_FLAG_RELEASE} /O1 /Ob1 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${EXA_MSVC_RUNTIME_FLAG_RELEASE} /Zi /O2 /Ob1 /DNDEBUG")

    if(EXA_MSVC_UNICODE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t /DUNICODE /D_UNICODE")
    endif()
endif()

if(EXA_USE_VCPKG)
    include_directories("$ENV{VCPKG_DIR}/installed/${VCPKG_TARGET_TRIPLET}/include")
endif()

add_subdirectory(exa)

if(EXA_BUILD_TESTS)
    add_subdirectory(exa_test)
endif()
