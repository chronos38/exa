include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../exa/include"
    SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/include"
)
set(SRCROOT "${CMAKE_CURRENT_SOURCE_DIR}/test")

set(
    SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/pch.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h"
    ${SRCROOT}/buffered_stream_test.cpp
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
    )
endif()

find_package(Threads)
add_executable(${PROJECT_NAME}_test ${SOURCES})
target_link_libraries(
    ${PROJECT_NAME}_test
    ${PROJECT_NAME}
    gtest
    gmock
    cryptopp
    Threads::Threads
)
