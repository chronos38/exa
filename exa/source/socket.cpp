#include <exa/socket.hpp>
#include <exa/enum_flag.hpp>
#include <exa/task.hpp>

#include <algorithm>

namespace exa
{
    bool socket::ipv4_supported_ = socket::protocol_supported(address_family::inter_network);
    bool socket::ipv6_supported_ = socket::protocol_supported(address_family::inter_network_v6);

    socket::socket(socket_type type, protocol_type protocol)
        : socket(ipv6_supported() ? address_family::inter_network_v6 : address_family::inter_network, type, protocol)
    {
    }

    socket::socket(address_family family, socket_type type, protocol_type protocol)
        : family_(family), type_(type), protocol_(protocol)
    {
        socket_ = ::socket(static_cast<std::underlying_type_t<address_family>>(family),
                           static_cast<std::underlying_type_t<socket_type>>(type),
                           static_cast<std::underlying_type_t<protocol_type>>(protocol));

#ifdef _WIN32
        if (socket_ == INVALID_SOCKET)
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "socket");
        }
#else
        if (socket_ == -1)
        {
            throw std::system_error(errno, std::system_category(), "socket");
        }
#endif
    }

    socket::socket(native_handle_type s, address_family family, protocol_type protocol)
    {
        socket_ = s;
        family_ = family;
        protocol_ = protocol;
        is_connected_ = true;
        type_ = static_cast<socket_type>(get_socket_option<int>(SOL_SOCKET, SO_TYPE));
    }

    socket::~socket()
    {
        close();
    }

    bool socket::valid() const
    {
        return is_valid_native_handle(socket_);
    }

    socket::native_handle_type socket::native_handle() const
    {
        validate_native_handle(socket_);
        return socket_;
    }

    bool socket::connected() const
    {
        return is_connected_;
    }

    bool socket::bound() const
    {
        return is_bound_;
    }

    address_family socket::family() const
    {
        validate_native_handle(socket_);
        return family_;
    }

    protocol_type socket::protocol() const
    {
        validate_native_handle(socket_);
        return protocol_;
    }

    socket_type socket::type() const
    {
        validate_native_handle(socket_);
        return type_;
    }

    size_t socket::available() const
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        u_long value = 0;
        if (ioctlsocket(socket_, FIONREAD, &value) != 0)
        {
            throw_error("ioctlsocket");
        }
#else
        int value = 0;
        if (ioctl(socket_, FIONREAD, &value) != 0)
        {
            throw_error("ioctl");
        }
#endif
        return static_cast<size_t>(std::max<decltype(value)>(0, value));
    }

    bool socket::blocking() const
    {
        validate_native_handle(socket_);
        return is_blocking_;
    }

    void socket::blocking(bool value)
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        u_long v = value ? 0 : 1;
        if (ioctlsocket(socket_, FIONBIO, &v) != 0)
        {
            throw_error("ioctlsocket");
        }
#else
        int v = value ? 0 : 1;
        if (ioctl(socket_, FIONBIO, &v) != 0)
        {
            throw_error("ioctlsocket");
        }
#endif
        is_blocking_ = value;
    }

    bool socket::dual_mode() const
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        auto v = get_socket_option<DWORD>(IPPROTO_IPV6, IPV6_V6ONLY);
#else
        auto v = get_socket_option<int>(IPPROTO_IPV6, IPV6_V6ONLY);
#endif
        return v == 0;
    }

    void socket::dual_mode(bool value)
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        DWORD v = value ? 0 : 1;
        set_socket_option(IPPROTO_IPV6, IPV6_V6ONLY, v);
#else
        int v = value ? 0 : 1;
        set_socket_option(IPPROTO_IPV6, IPV6_V6ONLY, v);
#endif
    }

    bool socket::enable_broadcast() const
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        auto v = get_socket_option<DWORD>(SOL_SOCKET, SO_BROADCAST);
#else
        auto v = get_socket_option<int>(SOL_SOCKET, SO_BROADCAST);
#endif
        return v != 0;
    }

    void socket::enable_broadcast(bool value)
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        DWORD v = value ? 1 : 0;
        set_socket_option(SOL_SOCKET, SO_BROADCAST, v);
#else
        int v = value ? 1 : 0;
        set_socket_option(SOL_SOCKET, SO_BROADCAST, v);
#endif
    }

    bool socket::exclusive_address_use() const
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        return get_socket_option<BOOL>(SOL_SOCKET, SO_EXCLUSIVEADDRUSE) != 0;
#else
        return false;
#endif
    }

    void socket::exclusive_address_use(bool value)
    {
#ifdef _WIN32
        BOOL v = value ? 1 : 0;
        set_socket_option(SOL_SOCKET, SO_EXCLUSIVEADDRUSE, v);
#endif
    }

    bool socket::reuse_address() const
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        auto v = get_socket_option<BOOL>(SOL_SOCKET, SO_REUSEADDR);
#else
        auto v = get_socket_option<int>(SOL_SOCKET, SO_REUSEADDR);
#endif
        return v != 0;
    }

    void socket::reuse_address(bool value)
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        BOOL v = value ? 1 : 0;
        set_socket_option(SOL_SOCKET, SO_REUSEADDR, v);
#else
        int v = value ? 1 : 0;
        set_socket_option(SOL_SOCKET, SO_REUSEADDR, v);
#endif
    }

    std::chrono::seconds socket::ttl() const
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        auto v = get_socket_option<DWORD>(IPPROTO_IP, IP_TTL);
#else
        auto v = get_socket_option<uint8_t>(IPPROTO_IP, IP_TTL);
#endif
        return std::chrono::seconds(v);
    }

    void socket::ttl(const std::chrono::seconds& value)
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        auto v = static_cast<DWORD>(value.count());
        set_socket_option(IPPROTO_IP, IP_TTL, v);
#else
        auto v = static_cast<uint8_t>(
            std::clamp<std::chrono::seconds::rep>(value.count(), 0, std::numeric_limits<uint8_t>::max()));
        set_socket_option(IPPROTO_IP, IP_TTL, v);
#endif
    }

    linger_option socket::linger_state() const
    {
        validate_native_handle(socket_);
        auto value = get_socket_option<linger>(SOL_SOCKET, SO_LINGER);
        return linger_option{value.l_onoff != 0, std::chrono::seconds(value.l_linger)};
    }

    void socket::linger_state(const linger_option& value)
    {
        validate_native_handle(socket_);
        linger v;
        v.l_linger = static_cast<decltype(linger::l_linger)>(value.linger_time.count());
        v.l_onoff = value.enabled ? 1 : 0;
        set_socket_option(SOL_SOCKET, SO_LINGER, v);
    }

    bool socket::no_delay() const
    {
        validate_native_handle(socket_);
        return get_socket_option<int>(IPPROTO_TCP, TCP_NODELAY) != 0;
    }

    void socket::no_delay(bool value)
    {
        validate_native_handle(socket_);
        set_socket_option(IPPROTO_TCP, TCP_NODELAY, value ? 1 : 0);
    }

    endpoint socket::local_endpoint() const
    {
        validate_native_handle(socket_);
        sockaddr_storage storage = {0};
#ifdef _WIN32
        int length = sizeof(storage);
#else
        socklen_t length = sizeof(storage);
#endif
        auto rc = getsockname(socket_, reinterpret_cast<sockaddr*>(&storage), &length);

        if (rc != 0)
        {
            throw_error("getsockname");
        }

        return endpoint(storage);
    }

    endpoint socket::remote_endpoint() const
    {
        validate_native_handle(socket_);
        sockaddr_storage storage = {0};
#ifdef _WIN32
        int length = sizeof(storage);
#else
        socklen_t length = sizeof(storage);
#endif
        auto rc = getpeername(socket_, reinterpret_cast<sockaddr*>(&storage), &length);

        if (rc != 0)
        {
            throw_error("getpeername");
        }

        return endpoint(storage);
    }

    size_t socket::send_buffer() const
    {
        validate_native_handle(socket_);
        return static_cast<size_t>(get_socket_option<int>(SOL_SOCKET, SO_SNDBUF));
    }

    void socket::send_buffer(size_t value)
    {
        validate_native_handle(socket_);
        set_socket_option(SOL_SOCKET, SO_SNDBUF, static_cast<int>(value));
    }

    size_t socket::receive_buffer() const
    {
        validate_native_handle(socket_);
        return static_cast<size_t>(get_socket_option<int>(SOL_SOCKET, SO_RCVBUF));
    }

    void socket::receive_buffer(size_t value)
    {
        validate_native_handle(socket_);
        set_socket_option(SOL_SOCKET, SO_RCVBUF, static_cast<int>(value));
    }

    std::chrono::milliseconds socket::send_timeout() const
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        return std::chrono::milliseconds(get_socket_option<int>(SOL_SOCKET, SO_SNDTIMEO));
#else
        auto timeout = get_socket_option<timeval>(SOL_SOCKET, SO_SNDTIMEO);
        auto s = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(timeout.tv_sec));
        auto us = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::microseconds(timeout.tv_usec));
        return s + us;
#endif
    }

    void socket::send_timeout(const std::chrono::milliseconds& value)
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        set_socket_option(SOL_SOCKET, SO_SNDTIMEO, static_cast<int>(value.count()));
#else
        timeval timeout;
        timeout.tv_sec =
            static_cast<decltype(timeout.tv_sec)>(std::chrono::duration_cast<std::chrono::seconds>(value).count());
        timeout.tv_usec = std::clamp<decltype(timeout.tv_usec)>(
            std::chrono::duration_cast<std::chrono::microseconds>(value).count(), -999999, 999999);
        set_socket_option(SOL_SOCKET, SO_SNDTIMEO, timeout);
#endif
    }

    std::chrono::milliseconds socket::receive_timeout() const
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        return std::chrono::milliseconds(get_socket_option<int>(SOL_SOCKET, SO_RCVTIMEO));
#else
        auto timeout = get_socket_option<timeval>(SOL_SOCKET, SO_RCVTIMEO);
        auto s = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(timeout.tv_sec));
        auto us = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::microseconds(timeout.tv_usec));
        return s + us;
#endif
    }

    void socket::receive_timeout(const std::chrono::milliseconds& value)
    {
        validate_native_handle(socket_);
#ifdef _WIN32
        set_socket_option(SOL_SOCKET, SO_RCVTIMEO, static_cast<int>(value.count()));
#else
        timeval timeout;
        timeout.tv_sec =
            static_cast<decltype(timeout.tv_sec)>(std::chrono::duration_cast<std::chrono::seconds>(value).count());
        timeout.tv_usec = std::clamp<decltype(timeout.tv_usec)>(
            std::chrono::duration_cast<std::chrono::microseconds>(value).count(), -999999, 999999);
        set_socket_option(SOL_SOCKET, SO_RCVTIMEO, timeout);
#endif
    }

    std::shared_ptr<socket> socket::accept() const
    {
        validate_native_handle(socket_);

        sockaddr_storage storage = {0};
#ifdef _WIN32
        int length = sizeof(storage);
#else
        socklen_t length = sizeof(storage);
#endif
        auto s = ::accept(socket_, reinterpret_cast<sockaddr*>(&storage), &length);

        if (!is_valid_native_handle(s))
        {
            throw_error("accept");
        }

        return std::make_shared<socket>(s, static_cast<address_family>(storage.ss_family), protocol_type::tcp);
    }

    std::future<std::shared_ptr<socket>> socket::accept_async() const
    {
        validate_native_handle(socket_);
        return task::run(std::bind(&socket::accept, this));
    }

    void socket::bind(const address& addr, uint16_t port)
    {
        bind(endpoint(addr, port));
    }

    void socket::bind(const endpoint& local_ep)
    {
        validate_native_handle(socket_);
        auto storage = local_ep.serialize();
        int rc = ::bind(socket_, reinterpret_cast<const sockaddr*>(storage.data()), static_cast<int>(storage.size()));

        if (rc != 0)
        {
            throw_error("bind");
        }
        else
        {
            is_bound_ = true;
        }
    }

    void socket::close()
    {
#ifdef _WIN32
        if (socket_ != INVALID_SOCKET)
        {
            closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }
#else
        if (socket_ != -1)
        {
            ::close(socket_);
            socket_ = -1;
        }
#endif
    }

    void socket::close(const std::chrono::seconds& wait_before_close)
    {
        validate_native_handle(socket_);
        linger_state(linger_option{true, wait_before_close});
        close();
    }

    void socket::connect(const endpoint& remote_ep)
    {
        validate_native_handle(socket_);
        auto storage = remote_ep.serialize();
        int rc = ::connect(socket_, reinterpret_cast<const sockaddr*>(storage.data()), static_cast<int>(storage.size()));

        if (rc != 0)
        {
            throw_error("connect");
        }
        else
        {
            is_connected_ = true;
        }
    }

    std::future<void> socket::connect_async(const endpoint& remote_ep)
    {
        validate_native_handle(socket_);
        return task::run([=] { return connect(remote_ep); });
    }

    void socket::connect(const address& addr, uint16_t port)
    {
        connect(endpoint(addr, port));
    }

    std::future<void> socket::connect_async(const address& addr, uint16_t port)
    {
        return connect_async(endpoint(addr, port));
    }

    void socket::connect(const std::string& host, uint16_t port)
    {
        auto endpoints = endpoint::get_address_info(host, std::to_string(port));
        connect(endpoints);
    }

    std::future<void> socket::connect_async(const std::string& host, uint16_t port)
    {
        return task::run([=] { connect(host, port); });
    }

    void socket::connect(gsl::span<const endpoint> endpoints)
    {
        for (auto&& ep : endpoints)
        {
            try
            {
                connect(ep);
            }
            catch (std::system_error&)
            {
                // Ignore failure for now.
            }

            if (is_connected_)
            {
                break;
            }
        }

        if (!is_connected_)
        {
            throw std::runtime_error("Couldn't connect to any given endpoint from collection.");
        }
    }

    std::future<void> socket::connect_async(gsl::span<const endpoint> endpoints)
    {
        return task::run([=] { connect(endpoints); });
    }

    void socket::listen(size_t backlog) const
    {
        validate_native_handle(socket_);
        auto rc = ::listen(socket_, static_cast<int>(backlog));

        if (rc != 0)
        {
            throw_error("listen");
        }
    }

    bool socket::poll(const std::chrono::microseconds& us, select_mode mode) const
    {
        validate_native_handle(socket_);

        fd_set set;
        FD_ZERO(&set);
        FD_SET(socket_, &set);

        timeval timeout;
        timeout.tv_sec =
            static_cast<decltype(timeval::tv_sec)>(std::chrono::duration_cast<std::chrono::seconds>(us).count());
        timeout.tv_usec =
            std::clamp<decltype(timeval::tv_usec)>(static_cast<decltype(timeval::tv_usec)>(us.count()), -999999, 999999);

#ifdef _WIN32
        int nfds = 0;
#else
        auto nfds = socket_ + 1;
#endif

        int rc = 0;

        switch (mode)
        {
            case select_mode::error:
                rc = ::select(nfds, nullptr, nullptr, &set, &timeout);
                break;
            case select_mode::read:
                rc = ::select(nfds, &set, nullptr, nullptr, &timeout);
                break;
            case select_mode::write:
                rc = ::select(nfds, nullptr, &set, nullptr, &timeout);
                break;
            default:
                break;
        }

        validate_transfer(rc, "select");
        return FD_ISSET(socket_, &set);
    }

    size_t socket::receive(gsl::span<uint8_t> buffer, socket_flags flags) const
    {
        validate_native_handle(socket_);

        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Receive buffer is null.");
        }

        auto n = recv(socket_, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()),
                      static_cast<std::underlying_type_t<socket_flags>>(flags));
        validate_transfer(n, "recv");
        return static_cast<size_t>(n);
    }

    std::future<size_t> socket::receive_async(gsl::span<uint8_t> buffer, socket_flags flags) const
    {
        validate_native_handle(socket_);
        return task::run([=] { return receive(buffer, flags); });
    }

    size_t socket::receive_from(gsl::span<uint8_t> buffer, endpoint& ep, socket_flags flags) const
    {
        validate_native_handle(socket_);

        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Receive buffer is null.");
        }

        sockaddr_storage storage = {0};
#ifdef _WIN32
        int length = sizeof(storage);
#else
        socklen_t length = sizeof(storage);
#endif
        auto n = recvfrom(socket_, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()),
                          static_cast<std::underlying_type_t<socket_flags>>(flags), reinterpret_cast<sockaddr*>(&storage),
                          &length);
        validate_transfer(n, "recvfrom");
        ep = endpoint(storage);
        return static_cast<size_t>(n);
    }

    std::future<socket_receive_from_result> socket::receive_from_async(gsl::span<uint8_t> buffer, socket_flags flags) const
    {
        validate_native_handle(socket_);
        return task::run([=] {
            endpoint ep;
            auto n = receive_from(buffer, ep, flags);
            return socket_receive_from_result{n, ep};
        });
    }

    size_t socket::send(gsl::span<const uint8_t> buffer, socket_flags flags) const
    {
        validate_native_handle(socket_);

        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Send buffer is null.");
        }

        auto n = ::send(socket_, reinterpret_cast<const char*>(buffer.data()), static_cast<int>(buffer.size()),
                        static_cast<std::underlying_type_t<socket_flags>>(flags));
        validate_transfer(n, "send");
        return static_cast<size_t>(n);
    }

    std::future<size_t> socket::send_async(gsl::span<const uint8_t> buffer, socket_flags flags) const
    {
        validate_native_handle(socket_);
        return task::run([=] { return send(buffer, flags); });
    }

    size_t socket::send_to(gsl::span<const uint8_t> buffer, const endpoint& ep, socket_flags flags) const
    {
        validate_native_handle(socket_);

        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Send buffer is null.");
        }

        auto addr = ep.serialize();
        auto n = sendto(socket_, reinterpret_cast<const char*>(buffer.data()), static_cast<int>(buffer.size()),
                        static_cast<std::underlying_type_t<socket_flags>>(flags),
                        reinterpret_cast<const sockaddr*>(addr.data()), static_cast<int>(addr.size()));
        validate_transfer(n, "sendto");
        return static_cast<size_t>(n);
    }

    std::future<size_t> socket::send_to_async(gsl::span<const uint8_t> buffer, const endpoint& ep, socket_flags flags) const
    {
        validate_native_handle(socket_);
        return task::run([=] { return send_to(buffer, ep, flags); });
    }

    void socket::shutdown(socket_shutdown flags) const
    {
        validate_native_handle(socket_);
        int how = 0;
#ifdef _WIN32
        if (has_flag(flags, socket_shutdown::both))
        {
            how = SD_BOTH;
        }
        else if (has_flag(flags, socket_shutdown::read))
        {
            how = SD_RECEIVE;
        }
        else if (has_flag(flags, socket_shutdown::write))
        {
            how = SD_SEND;
        }
#else
        if (has_flag(flags, socket_shutdown::both))
        {
            how = SHUT_RDWR;
        }
        else if (has_flag(flags, socket_shutdown::read))
        {
            how = SHUT_RD;
        }
        else if (has_flag(flags, socket_shutdown::write))
        {
            how = SHUT_WR;
        }
#endif
        auto rc = ::shutdown(socket_, how);

        if (rc != 0)
        {
            throw_error("shutdown");
        }
    }

    void socket::select(std::vector<std::shared_ptr<socket>>& read, std::vector<std::shared_ptr<socket>>& write,
                        std::vector<std::shared_ptr<socket>>& error, const std::chrono::microseconds& us)
    {
        timeval timeout;
        timeout.tv_sec =
            static_cast<decltype(timeval::tv_sec)>(std::chrono::duration_cast<std::chrono::seconds>(us).count());
        timeout.tv_usec =
            std::clamp<decltype(timeval::tv_usec)>(static_cast<decltype(timeval::tv_usec)>(us.count()), -999999, 999999);

        fd_set readset;
        FD_ZERO(&readset);
        fd_set writeset;
        FD_ZERO(&writeset);
        fd_set exceptset;
        FD_ZERO(&exceptset);

        int nfds = -1;

        auto fdset = [&nfds](std::vector<std::shared_ptr<socket>>& v, fd_set& set) {
            for (auto& s : v)
            {
                FD_SET(s->socket_, &set);
#ifndef _WIN32
                nfds = std::max<int>(s->socket_, nfds);
#endif
            }
        };

        fdset(read, readset);
        fdset(write, writeset);
        fdset(error, exceptset);
        auto rc = ::select(nfds + 1, &readset, &writeset, &exceptset, &timeout);
        validate_transfer(rc, "select");

        auto result = [](std::vector<std::shared_ptr<socket>>& v, fd_set& set) {
            for (auto it = std::begin(v); it != std::end(v);)
            {
                auto& s = *it;

                if (FD_ISSET(s->socket_, &set))
                {
                    ++it;
                }
                else
                {
                    it = v.erase(it);
                }
            }
        };

        result(read, readset);
        result(write, writeset);
        result(error, exceptset);
    }

    bool socket::protocol_supported(address_family family)
    {
#ifdef _WIN32
        auto s = WSASocketW(static_cast<std::underlying_type_t<address_family>>(family), SOCK_DGRAM, 0, nullptr, 0, 0);

        if (s == INVALID_SOCKET)
        {
            return false;
        }
        else
        {
            closesocket(s);
            return true;
        }
#else
        auto s = ::socket(static_cast<std::underlying_type_t<address_family>>(family), SOCK_DGRAM, 0);

        if (s == -1)
        {
            return errno != EAFNOSUPPORT;
        }
        else
        {
            ::close(s);
            return true;
        }
#endif
    }

    bool socket::ipv4_supported()
    {
        return ipv4_supported_;
    }

    bool socket::ipv6_supported()
    {
        return ipv6_supported_;
    }

    bool socket::is_valid_native_handle(native_handle_type s)
    {
#ifdef _WIN32
        return s != INVALID_SOCKET;
#else
        return s != -1;
#endif
    }

    void socket::validate_native_handle(native_handle_type s)
    {
#ifdef _WIN32
        if (s == INVALID_SOCKET)
        {
            throw std::runtime_error("Socket is invalid.");
        }
#else
        if (s == -1)
        {
            throw std::runtime_error("Socket is invalid.");
        }
#endif
    }

    void socket::validate_transfer(int rc, const std::string& message)
    {
#ifdef _WIN32
        if (rc == SOCKET_ERROR)
        {
            throw_error(message);
        }
#else
        if (rc == -1)
        {
            throw_error(message);
        }
#endif
    }

    void socket::throw_error(const std::string& message)
    {
#ifdef _WIN32
        throw std::system_error(WSAGetLastError(), std::system_category(), message);
#else
        throw std::system_error(errno, std::system_category(), message);
#endif
    }
}
