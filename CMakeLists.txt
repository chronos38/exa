cmake_minimum_required(VERSION 3.9.0)
project(exa VERSION 0.1)

option(EXA_BUILD_TESTS "Build test cases." ON)

set(INCROOT "${CMAKE_CURRENT_SOURCE_DIR}/exa/include/exa")
set(SRCROOT "${CMAKE_CURRENT_SOURCE_DIR}/exa/source")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/exa/include")

set(
  SOURCES
  ${INCROOT}/address.hpp
  ${SRCROOT}/address.cpp
  ${INCROOT}/buffered_stream.hpp
  ${SRCROOT}/buffered_stream.cpp
  ${INCROOT}/dependencies.hpp
  ${INCROOT}/enum_flag.hpp
  ${INCROOT}/endpoint.hpp
  ${SRCROOT}/endpoint.cpp
  ${INCROOT}/file_stream.hpp
  ${SRCROOT}/file_stream.unix.cpp
  ${SRCROOT}/file_stream.win32.cpp
  ${INCROOT}/lockable.hpp
  ${INCROOT}/memory_stream.hpp
  ${SRCROOT}/memory_stream.cpp
  ${INCROOT}/network_stream.hpp
  ${SRCROOT}/network_stream.cpp
  ${INCROOT}/socket_base.hpp
  ${INCROOT}/socket.hpp
  ${SRCROOT}/socket.cpp
  ${INCROOT}/stream.hpp
  ${SRCROOT}/stream.cpp
  ${INCROOT}/task.hpp
  ${SRCROOT}/task.cpp
  ${INCROOT}/tcp_client.hpp
  ${SRCROOT}/tcp_client.cpp
  ${INCROOT}/tcp_listener.hpp
  ${SRCROOT}/tcp_listener.cpp
  ${INCROOT}/udp_client.hpp
  ${SRCROOT}/udp_client.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if(EXA_BUILD_TESTS)
  find_library(gtest NAMES gtest googletest)
  find_library(gtest_main NAMES gtest_main googletest_main)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/exa_test/include")
  set(SRCROOT "${CMAKE_CURRENT_SOURCE_DIR}/exa_test/test")

  set(
    SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/exa_test/source/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exa_test/source/pch.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exa_test/include/pch.h"
    ${SRCROOT}/buffered_stream_test.cpp
    ${SRCROOT}/file_stream_test.cpp
    ${SRCROOT}/network_stream_test.cpp
    ${SRCROOT}/task_test.cpp
    ${SRCROOT}/tcp_client_test.cpp
    ${SRCROOT}/tcp_listener_test.cpp
    ${SRCROOT}/udp_client_test.cpp
  )

  add_executable(${PROJECT_NAME}_test ${SOURCES})
  target_link_libraries(
    ${PROJECT_NAME}_test
    ${PROJECT_NAME}
    ${gtest_main}
    ${gtest}
    pthread
  )
endif()
