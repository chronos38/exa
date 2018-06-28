
set(SRCROOT "${CMAKE_CURRENT_SOURCE_DIR}/test")

set(
    SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/pch.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h"
    ${SRCROOT}/buffered_stream_test.cpp
    ${SRCROOT}/crypto_stream_test.cpp
    ${SRCROOT}/file_stream_test.cpp
    ${SRCROOT}/network_stream_test.cpp
    ${SRCROOT}/task_test.cpp
    ${SRCROOT}/tcp_client_test.cpp
    ${SRCROOT}/tcp_listener_test.cpp
    ${SRCROOT}/udp_client_test.cpp
)

if(WIN32)
    add_definitions(
        -D_UNICODE
        -DUNICODE
        -D_CRT_SECURE_NO_WARNINGS
    )
endif()

find_package(Threads)
find_package(gtest)
find_package(cryptopp)
find_package(gsl)
add_executable(${PROJECT_NAME}_test ${SOURCES})

target_include_directories(
    ${PROJECT_NAME}_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../exa/include"
)

target_link_libraries(
    ${PROJECT_NAME}_test
    ${PROJECT_NAME}
    gtest
    gmock
    cryptopp
    Threads::Threads
)
