set(INCROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/exa")
set(SRCROOT "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(
    SOURCES
    ${INCROOT}/address.hpp
    ${SRCROOT}/address.cpp
    ${INCROOT}/buffered_stream.hpp
    ${SRCROOT}/buffered_stream.cpp
    ${INCROOT}/concepts.hpp
    ${INCROOT}/crypto_block_cipher.hpp
    ${INCROOT}/crypto_stream.hpp
    ${SRCROOT}/crypto_stream.cpp
    ${INCROOT}/crypto_transform.hpp
    ${INCROOT}/dependencies.hpp
    ${INCROOT}/endpoint.hpp
    ${SRCROOT}/endpoint.cpp
    ${INCROOT}/enum_flag.hpp
    ${INCROOT}/file_stream.hpp
    ${SRCROOT}/file_stream.unix.cpp
    ${SRCROOT}/file_stream.win32.cpp
    ${INCROOT}/memory_stream.hpp
    ${SRCROOT}/memory_stream.cpp
    ${INCROOT}/network_stream.hpp
    ${SRCROOT}/network_stream.cpp
    ${INCROOT}/socket_base.hpp
    ${INCROOT}/socket.hpp
    ${SRCROOT}/socket.cpp
    ${INCROOT}/stream.hpp
    ${SRCROOT}/stream.cpp
    ${INCROOT}/symmetric_algorithm.hpp
    ${SRCROOT}/symmetric_algorithm.cpp
    ${INCROOT}/task.hpp
    ${SRCROOT}/task.cpp
    ${INCROOT}/tcp_client.hpp
    ${SRCROOT}/tcp_client.cpp
    ${INCROOT}/tcp_listener.hpp
    ${SRCROOT}/tcp_listener.cpp
    ${INCROOT}/udp_client.hpp
    ${SRCROOT}/udp_client.cpp
)

if(WIN32)
    add_definitions(
        -D_UNICODE
        -DUNICODE
    )
endif()

find_package(cryptopp)
find_package(gsl)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(
    ${PROJECT_NAME}_test
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
