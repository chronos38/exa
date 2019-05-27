set(INCROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/exa")
set(SRCROOT "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(
    SOURCES
    # public interface files
    ${INCROOT}/address.hpp
    ${INCROOT}/buffered_stream.hpp
    ${INCROOT}/dependencies.hpp
    ${INCROOT}/concepts.hpp
    ${INCROOT}/endpoint.hpp
    ${INCROOT}/enum_flag.hpp
    ${INCROOT}/file_stream.hpp
    ${INCROOT}/memory_stream.hpp
    ${INCROOT}/network_stream.hpp
    ${INCROOT}/socket_base.hpp
    ${INCROOT}/socket.hpp
    ${INCROOT}/stream.hpp
    ${INCROOT}/task.hpp
    ${INCROOT}/tcp_client.hpp
    ${INCROOT}/tcp_listener.hpp
    ${INCROOT}/udp_client.hpp
    # source files
    ${SRCROOT}/address.cpp
    ${SRCROOT}/buffered_stream.cpp
    ${SRCROOT}/endpoint.cpp
    ${SRCROOT}/file_stream.unix.cpp
    ${SRCROOT}/file_stream.win32.cpp
    ${SRCROOT}/memory_stream.cpp
    ${SRCROOT}/network_stream.cpp
    ${SRCROOT}/socket.cpp
    ${SRCROOT}/stream.cpp
    ${SRCROOT}/task.cpp
    ${SRCROOT}/tcp_client.cpp
    ${SRCROOT}/tcp_listener.cpp
    ${SRCROOT}/udp_client.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
