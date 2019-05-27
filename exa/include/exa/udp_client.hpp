#pragma once

#include <exa/socket.hpp>

#include <memory>
#include <future>
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

namespace exa
{
    struct udp_receive_result
    {
        std::vector<uint8_t> buffer;
        endpoint endpoint;
    };

    class udp_client
    {
    public:
        udp_client();
        udp_client(const udp_client&) = delete;
        explicit udp_client(address_family family);
        explicit udp_client(uint16_t port);
        udp_client(uint16_t port, address_family family);
        explicit udp_client(const endpoint& local_ep);
        udp_client(const std::string& host, uint16_t port);
        ~udp_client() = default;

        bool connected() const;
        size_t available() const;
        bool enable_broadcast() const;
        void enable_broadcast(bool value);
        bool exclusive_address_use() const;
        void exclusive_address_use(bool value);
        bool reuse_address() const;
        void reuse_address(bool value);
        std::chrono::seconds ttl() const;
        void ttl(const std::chrono::seconds& value);
        const std::shared_ptr<socket>& socket() const;

        void close();
        void connect(const address& addr, uint16_t port);
        void connect(const endpoint& remote_ep);
        void connect(const std::string& host, uint16_t port);
        std::vector<uint8_t> receive(endpoint& ep);
        std::future<udp_receive_result> receive_async();
        size_t send(gsl::span<const uint8_t> buffer);
        std::future<size_t> send_async(gsl::span<const uint8_t> buffer);
        size_t send(gsl::span<const uint8_t> buffer, const endpoint& ep);
        std::future<size_t> send_async(gsl::span<const uint8_t> buffer, const endpoint& ep);

    private:
        static constexpr size_t max_udp_size = 0x10000;

        std::vector<uint8_t> buffer_;
        std::shared_ptr<exa::socket> socket_;
    };
}
