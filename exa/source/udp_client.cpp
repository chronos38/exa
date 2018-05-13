#include <exa/udp_client.hpp>
#include <exa/task.hpp>

namespace exa
{
    udp_client::udp_client() : udp_client(address_family::inter_network)
    {
    }

    udp_client::udp_client(address_family family) : buffer_(max_udp_size)
    {
        if (family != address_family::inter_network && family != address_family::inter_network_v6)
        {
            throw std::invalid_argument("UDP client needs IPv4 or IPv6 address family.");
        }

        socket_ = std::make_shared<exa::socket>(family, socket_type::datagram, protocol_type::udp);
    }

    udp_client::udp_client(uint16_t port) : udp_client(port, address_family::inter_network)
    {
    }

    udp_client::udp_client(uint16_t port, address_family family) : udp_client(family)
    {
        switch (family)
        {
            case exa::address_family::inter_network:
                socket_->bind(endpoint(address::any, port));
                break;
            case exa::address_family::inter_network_v6:
                socket_->bind(endpoint(address::ipv6_any, port));
                break;
            default:
                throw std::invalid_argument("UDP client needs IPv4 or IPv6 address family.");
        }
    }

    udp_client::udp_client(const endpoint& local_ep) : udp_client(local_ep.family())
    {
        socket_->bind(local_ep);
    }

    udp_client::udp_client(const std::string& host, uint16_t port) : buffer_(max_udp_size)
    {
        std::shared_ptr<exa::socket> ipv6;
        std::shared_ptr<exa::socket> ipv4;

        if (socket::ipv4_supported())
        {
            ipv4 = std::make_shared<exa::socket>(address_family::inter_network, socket_type::datagram, protocol_type::udp);
        }
        if (socket::ipv6_supported())
        {
            ipv6 =
                std::make_shared<exa::socket>(address_family::inter_network_v6, socket_type::datagram, protocol_type::udp);
        }

        if (!ipv4 && !ipv6)
        {
            throw std::runtime_error("Couldn't create sockets for UDP client.");
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
            throw std::runtime_error("UDP client coulnd't connect to given host.");
        }
    }

    bool udp_client::connected() const
    {
        return socket_->connected();
    }

    size_t udp_client::available() const
    {
        return socket_->available();
    }

    bool udp_client::enable_broadcast() const
    {
        return socket_->enable_broadcast();
    }

    void udp_client::enable_broadcast(bool value)
    {
        socket_->enable_broadcast(value);
    }

    bool udp_client::exclusive_address_use() const
    {
        return socket_->exclusive_address_use();
    }

    void udp_client::exclusive_address_use(bool value)
    {
        socket_->exclusive_address_use(value);
    }

    bool udp_client::reuse_address() const
    {
        return socket_->reuse_address();
    }

    void udp_client::reuse_address(bool value)
    {
        socket_->reuse_address(value);
    }

    std::chrono::seconds udp_client::ttl() const
    {
        return socket_->ttl();
    }

    void udp_client::ttl(const std::chrono::seconds& value)
    {
        socket_->ttl(value);
    }

    const std::shared_ptr<socket>& udp_client::socket() const
    {
        return socket_;
    }

    void udp_client::close()
    {
        socket_->close();
    }

    void udp_client::connect(const endpoint& remote_ep)
    {
        socket_->connect(remote_ep);
    }

    std::vector<uint8_t> udp_client::receive(endpoint& ep)
    {
        auto n = socket_->receive_from(buffer_, ep);

        if (n < max_udp_size)
        {
            auto first = std::begin(buffer_);
            auto last = std::next(first, n);
            return std::vector<uint8_t>(first, last);
        }
        else
        {
            return buffer_;
        }
    }

    std::future<udp_receive_result> udp_client::receive_async()
    {
        return task::run([this] {
            endpoint ep;
            std::vector<uint8_t> b(max_udp_size);
            auto n = socket_->receive_from(b, ep);
            b.resize(n);
            return udp_receive_result{b, ep};
        });
    }

    size_t udp_client::send(gsl::span<const uint8_t> buffer)
    {
        return socket_->send(buffer);
    }

    std::future<size_t> udp_client::send_async(gsl::span<const uint8_t> buffer)
    {
        return task::run([=] { return socket_->send(buffer); });
    }

    size_t udp_client::send(gsl::span<const uint8_t> buffer, const endpoint& ep)
    {
        return socket_->send_to(buffer, ep);
    }

    std::future<size_t> udp_client::send_async(gsl::span<const uint8_t> buffer, const endpoint& ep)
    {
        return task::run([=] { return socket_->send_to(buffer, ep); });
    }
}
