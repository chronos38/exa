#pragma once

#include <exa/socket.hpp>
#include <exa/network_stream.hpp>

#include <memory>
#include <future>
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

namespace exa
{
    class tcp_client
    {
    public:
        tcp_client();
        tcp_client(const tcp_client&) = delete;
        explicit tcp_client(address_family family);
        explicit tcp_client(const endpoint& local_ep);
        tcp_client(const address& addr, uint16_t port);
        tcp_client(const std::string& host, uint16_t port);
        explicit tcp_client(std::unique_ptr<exa::socket>&& s);
        ~tcp_client() = default;

        bool connected() const;
        size_t available() const;
        bool exclusive_address_use() const;
        void exclusive_address_use(bool value);
        bool reuse_address() const;
        void reuse_address(bool value);
        linger_option linger_state() const;
        void linger_state(const linger_option& value);
        bool no_delay() const;
        void no_delay(bool value);
        size_t send_buffer() const;
        void send_buffer(size_t value);
        size_t receive_buffer() const;
        void receive_buffer(size_t value);
        std::chrono::milliseconds send_timeout() const;
        void send_timeout(const std::chrono::milliseconds& value);
        std::chrono::milliseconds receive_timeout() const;
        void receive_timeout(const std::chrono::milliseconds& value);
        socket* socket() const;

        void close();
        void connect(const endpoint& remote_ep);
        std::future<void> connect_async(const endpoint& remote_ep);
        void connect(const address& addr, uint16_t port);
        std::future<void> connect_async(const address& addr, uint16_t port);
        void connect(const std::string& host, uint16_t port);
        std::future<void> connect_async(const std::string& host, uint16_t port);
        void connect(gsl::span<const endpoint> endpoints);
        std::future<void> connect_async(gsl::span<const endpoint> endpoints);
        network_stream* stream();

    private:
        std::unique_ptr<exa::socket> socket_;
        std::unique_ptr<exa::network_stream> stream_;
    };
}
