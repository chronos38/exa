
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

find_package(Cpp-Taskflow CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(gsl)

enable_testing()

add_executable(${PROJECT_NAME}_test ${SOURCES})

target_include_directories(
    ${PROJECT_NAME}_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../exa/include"
)

target_link_libraries(
    ${PROJECT_NAME}_test
    PUBLIC ${PROJECT_NAME}
	PRIVATE Cpp-Taskflow::Cpp-Taskflow
    PRIVATE GTest::GTest
    PRIVATE GTest::Main
    PRIVATE Threads::Threads
)

add_test(AllTestsInMain ${PROJECT_NAME}_test)
