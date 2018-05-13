#include <exa/tcp_client.hpp>

namespace exa
{
    tcp_client::tcp_client() : tcp_client(address_family::inter_network)
    {
    }

    tcp_client::tcp_client(address_family family)
    {
        if (family != address_family::inter_network && family != address_family::inter_network_v6)
        {
            throw std::invalid_argument("TCP client has to use either IPv4 or IPv6.");
        }

        socket_ = std::make_shared<exa::socket>(family, socket_type::stream, protocol_type::tcp);
    }

    tcp_client::tcp_client(const endpoint& local_ep) : tcp_client(local_ep.family())
    {
        socket_->bind(local_ep);
    }

    tcp_client::tcp_client(const address& addr, uint16_t port) : tcp_client(endpoint(addr, port))
    {
    }

    tcp_client::tcp_client(const std::string& host, uint16_t port)
    {
        std::shared_ptr<exa::socket> ipv6;
        std::shared_ptr<exa::socket> ipv4;

        if (socket::ipv4_supported())
        {
            ipv4 = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
        }
        if (socket::ipv6_supported())
        {
            ipv6 = std::make_shared<exa::socket>(address_family::inter_network_v6, socket_type::stream, protocol_type::tcp);
        }

        if (!ipv4 && !ipv6)
        {
            throw std::runtime_error("Couldn't create sockets for TCP client.");
        }

        auto endpoints = endpoint::get_address_info(host, std::to_string(port));

        for (auto& ep : endpoints)
        {
            switch (ep.family())
            {
                case address_family::inter_network:
                    if (ipv4)
                    {
                        ipv4->connect(ep);

                        if (ipv6)
                        {
                            ipv6->close();
                        }

                        socket_ = ipv4;
                    }
                    break;
                case address_family::inter_network_v6:
                    if (ipv6)
                    {
                        ipv6->connect(ep);

                        if (ipv4)
                        {
                            ipv4->close();
                        }

                        socket_ = ipv6;
                    }
                    break;
                default:
                    break;
            }

            if (socket_)
            {
                break;
            }
        }

        if (!socket_)
        {
            throw std::runtime_error("TCP client coulnd't connect to given host.");
        }
    }

    tcp_client::tcp_client(const std::shared_ptr<exa::socket>& s)
    {
        if (s == nullptr)
        {
            throw std::invalid_argument("Socket for TCP client is nullptr.");
        }
        if (s->type() != socket_type::stream)
        {
            throw std::invalid_argument("TCP client works only for stream sockets.");
        }
        if (s->protocol() != protocol_type::tcp)
        {
            throw std::invalid_argument("TCP client works only with TCP protocol.");
        }

        socket_ = s;
    }

    bool tcp_client::connected() const
    {
        return socket_->connected();
    }

    size_t tcp_client::available() const
    {
        return socket_->available();
    }

    bool tcp_client::exclusive_address_use() const
    {
        return socket_->exclusive_address_use();
    }

    void tcp_client::exclusive_address_use(bool value)
    {
        socket_->exclusive_address_use(value);
    }

    bool tcp_client::reuse_address() const
    {
        return socket_->reuse_address();
    }

    void tcp_client::reuse_address(bool value)
    {
        socket_->reuse_address(value);
    }

    linger_option tcp_client::linger_state() const
    {
        return socket_->linger_state();
    }

    void tcp_client::linger_state(const linger_option& value)
    {
        socket_->linger_state(value);
    }

    bool tcp_client::no_delay() const
    {
        return socket_->no_delay();
    }

    void tcp_client::no_delay(bool value)
    {
        socket_->no_delay(value);
    }

    size_t tcp_client::send_buffer() const
    {
        return socket_->send_buffer();
    }

    void tcp_client::send_buffer(size_t value)
    {
        socket_->send_buffer(value);
    }

    size_t tcp_client::receive_buffer() const
    {
        return socket_->receive_buffer();
    }

    void tcp_client::receive_buffer(size_t value)
    {
        socket_->receive_buffer(value);
    }

    std::chrono::milliseconds tcp_client::send_timeout() const
    {
        return socket_->send_timeout();
    }

    void tcp_client::send_timeout(const std::chrono::milliseconds& value)
    {
        socket_->send_timeout(value);
    }

    std::chrono::milliseconds tcp_client::receive_timeout() const
    {
        return socket_->receive_timeout();
    }

    void tcp_client::receive_timeout(const std::chrono::milliseconds& value)
    {
        socket_->receive_timeout(value);
    }

    const std::shared_ptr<socket>& tcp_client::socket() const
    {
        return socket_;
    }

    void tcp_client::close()
    {
        socket_->close();
    }

    void tcp_client::connect(const endpoint& ep)
    {
        socket_->connect(ep);
    }

    std::future<void> tcp_client::connect_async(const endpoint& ep)
    {
        return socket_->connect_async(ep);
    }

    void tcp_client::connect(const address& addr, uint16_t port)
    {
        socket_->connect(addr, port);
    }

    std::future<void> tcp_client::connect_async(const address& addr, uint16_t port)
    {
        return socket_->connect_async(addr, port);
    }

    void tcp_client::connect(const std::string& host, uint16_t port)
    {
        socket_->connect(host, port);
    }

    std::future<void> tcp_client::connect_async(const std::string& host, uint16_t port)
    {
        return socket_->connect_async(host, port);
    }

    void tcp_client::connect(gsl::span<const endpoint> endpoints)
    {
        socket_->connect(endpoints);
    }

    std::future<void> tcp_client::connect_async(gsl::span<const endpoint> endpoints)
    {
        return socket_->connect_async(endpoints);
    }

    const std::shared_ptr<network_stream>& tcp_client::stream()
    {
        if (!socket_->connected())
        {
            throw std::runtime_error("Can't retrieve stream for unconnected client.");
        }
        if (stream_ == nullptr)
        {
            stream_ = std::make_shared<network_stream>(socket_, true);
        }

        return stream_;
    }
}
