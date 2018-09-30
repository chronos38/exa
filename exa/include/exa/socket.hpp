#pragma once

#include <exa/dependencies.hpp>
#include <exa/socket_base.hpp>
#include <exa/endpoint.hpp>
#include <exa/address.hpp>

#include <chrono>
#include <vector>
#include <string>
#include <future>

namespace exa
{
    struct linger_option
    {
        bool enabled = false;
        std::chrono::seconds linger_time;
    };

    struct socket_receive_from_result
    {
        size_t bytes = 0;
        endpoint endpoint;
    };

    class socket
    {
    public:
#ifdef _WIN32
        using native_handle_type = SOCKET;
#else
        typedef int native_handle_type;
#endif

        socket() = delete;
        socket(const socket&) = delete;
        socket(socket_type type, protocol_type protocol);
        socket(address_family family, socket_type type, protocol_type protocol);
        socket(native_handle_type s, address_family family, protocol_type protocol);

        ~socket();

        bool valid() const;
        native_handle_type native_handle() const;
        bool connected() const;
        bool bound() const;
        address_family family() const;
        protocol_type protocol() const;
        socket_type type() const;
        size_t available() const;
        bool blocking() const;
        void blocking(bool value);
        bool dual_mode() const;
        void dual_mode(bool value);
        bool enable_broadcast() const;
        void enable_broadcast(bool value);
        bool exclusive_address_use() const;
        void exclusive_address_use(bool value);
        bool reuse_address() const;
        void reuse_address(bool value);
        std::chrono::seconds ttl() const;
        void ttl(const std::chrono::seconds& value);
        linger_option linger_state() const;
        void linger_state(const linger_option& value);
        bool no_delay() const;
        void no_delay(bool value);
        endpoint local_endpoint() const;
        endpoint remote_endpoint() const;
        size_t send_buffer() const;
        void send_buffer(size_t value);
        size_t receive_buffer() const;
        void receive_buffer(size_t value);
        std::chrono::milliseconds send_timeout() const;
        void send_timeout(const std::chrono::milliseconds& value);
        std::chrono::milliseconds receive_timeout() const;
        void receive_timeout(const std::chrono::milliseconds& value);

        std::shared_ptr<socket> accept() const;
        std::future<std::shared_ptr<socket>> accept_async() const;
        void bind(const address& addr, uint16_t port);
        void bind(const endpoint& local_ep);
        void close();
        void close(const std::chrono::seconds& wait_before_close);
        void connect(const endpoint& remote_ep);
        std::future<void> connect_async(const endpoint& remote_ep);
        void connect(const address& addr, uint16_t port);
        std::future<void> connect_async(const address& addr, uint16_t port);
        void connect(const std::string& host, uint16_t port);
        std::future<void> connect_async(const std::string& host, uint16_t port);
        void connect(gsl::span<const endpoint> endpoints);
        std::future<void> connect_async(gsl::span<const endpoint> endpoints);
        void listen(size_t backlog) const;
        bool poll(const std::chrono::microseconds& us, select_mode mode) const;
        size_t receive(gsl::span<uint8_t> buffer, socket_flags flags = socket_flags::none) const;
        std::future<size_t> receive_async(gsl::span<uint8_t> buffer, socket_flags flags = socket_flags::none) const;
        size_t receive_from(gsl::span<uint8_t> buffer, endpoint& ep, socket_flags flags = socket_flags::none) const;
        std::future<socket_receive_from_result> receive_from_async(gsl::span<uint8_t> buffer,
                                                                   socket_flags flags = socket_flags::none) const;
        size_t send(gsl::span<const uint8_t> buffer, socket_flags flags = socket_flags::none) const;
        std::future<size_t> send_async(gsl::span<const uint8_t> buffer, socket_flags flags = socket_flags::none) const;
        size_t send_to(gsl::span<const uint8_t> buffer, const endpoint& ep, socket_flags flags = socket_flags::none) const;
        std::future<size_t> send_to_async(gsl::span<const uint8_t> buffer, const endpoint& ep,
                                          socket_flags flags = socket_flags::none) const;
        void shutdown(socket_shutdown flags) const;

        static void select(std::vector<std::shared_ptr<socket>>& read, std::vector<std::shared_ptr<socket>>& write,
                           std::vector<std::shared_ptr<socket>>& error, const std::chrono::microseconds& us);
        template <class T>
        T get_socket_option(int level, int option) const
        {
            T value = T();
#ifdef _WIN32
            int length = sizeof(T);
            auto rc = getsockopt(socket_, level, option, reinterpret_cast<char*>(&value), &length);
#else
            socklen_t length = sizeof(T);
            auto rc = getsockopt(socket_, level, option, reinterpret_cast<int*>(&value), &length);
#endif

            if (rc != 0)
            {
                throw_error("getsockopt");
            }

            return value;
        }

        template <class T>
        void set_socket_option(int level, int option, const T& value) const
        {
#ifdef _WIN32
            auto rc = setsockopt(socket_, level, option, reinterpret_cast<const char*>(&value), sizeof(T));
#else
            auto rc =
                setsockopt(socket_, level, option, reinterpret_cast<const int*>(&value), static_cast<socklen_t>(sizeof(T)));
#endif

            if (rc != 0)
            {
                throw_error("setsockopt");
            }
        }

        static bool protocol_supported(address_family family);
        static bool ipv4_supported();
        static bool ipv6_supported();

    private:
        static bool is_valid_native_handle(native_handle_type s);
        static void validate_native_handle(native_handle_type s);
        static void validate_transfer(int rc, const std::string& message);
        static void throw_error(const std::string& message);

        address_family family_;
        socket_type type_;
        protocol_type protocol_;
        native_handle_type socket_;
        bool is_connected_ = false;
        bool is_bound_ = false;
        bool is_blocking_ = true;

        static bool ipv4_supported_;
        static bool ipv6_supported_;
    };
}
