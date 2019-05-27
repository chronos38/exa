#pragma once

#include <exa/socket.hpp>
#include <exa/tcp_client.hpp>

#include <memory>
#include <future>
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

namespace exa
{
    class tcp_listener
    {
    public:
        static constexpr size_t max_connections = 0x7fffffff;

        tcp_listener() = delete;
        tcp_listener(const tcp_listener&) = delete;
        explicit tcp_listener(uint16_t port);
        tcp_listener(const address& addr, uint16_t port);
        explicit tcp_listener(const endpoint& ep);
        ~tcp_listener();

        bool active() const;
        bool exclusive_address_use() const;
        void exclusive_address_use(bool value);
        bool reuse_address() const;
        void reuse_address(bool value);
        endpoint local_endpoint() const;
        const std::unique_ptr<socket>& socket() const;

        std::unique_ptr<exa::socket> accept_socket() const;
        std::future<std::unique_ptr<exa::socket>> accept_socket_async() const;
        std::unique_ptr<tcp_client> accept_client() const;
        std::future<std::unique_ptr<tcp_client>> accept_client_async() const;

        bool pending() const;
        void start(size_t backlog = 0x7fffffff);
        void stop();

        static std::unique_ptr<tcp_listener> create(uint16_t port);

    private:
        endpoint endpoint_;
        std::unique_ptr<exa::socket> socket_;
        bool active_ = false;
    };
}
