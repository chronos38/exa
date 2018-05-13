#include <exa/address.hpp>

#include <array>
#include <iterator>

namespace exa
{
    const address address::any(0);
    const address address::loopback(0x7f000001);
    const address address::broadcast(0xffffffff);
    const address address::ipv6_any({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0);
    const address address::ipv6_loopback({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 0);
    const address address::ipv6_broadcast({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0);

    address::address() : address(0)
    {
    }

    address::address(uint32_t ipv4)
    {
        ipv4 = htonl(ipv4);
        auto buffer = reinterpret_cast<uint8_t*>(&ipv4);
        address_.insert(std::end(address_), buffer, buffer + sizeof(ipv4));
        family_ = address_family::inter_network;
    }

    address::address(gsl::span<const uint8_t> ipv6, uint32_t scope_id)
    {
        if (ipv6.size() != 16)
        {
            throw std::out_of_range("Given IPv6 address has to be exactly 16 bytes long.");
        }

        address_.insert(std::end(address_), std::begin(ipv6), std::end(ipv6));
        scope_id_ = scope_id;
        family_ = address_family::inter_network_v6;
    }

    address::address(std::initializer_list<const uint8_t> ipv6, uint32_t scope_id)
    {
        if (ipv6.size() != 16)
        {
            throw std::out_of_range("Given IPv6 address has to be exactly 16 bytes long.");
        }

        address_.insert(std::end(address_), std::begin(ipv6), std::end(ipv6));
        scope_id_ = scope_id;
        family_ = address_family::inter_network_v6;
    }

    const std::vector<uint8_t>& address::bytes() const
    {
        return address_;
    }

    address_family address::family() const
    {
        return family_;
    }

    uint32_t address::scope_id() const
    {
        return scope_id_;
    }

    void address::scope_id(uint32_t value)
    {
        scope_id_ = value;
    }

    std::string address::to_string() const
    {
        char str[INET6_ADDRSTRLEN] = {0};
        memset(str, 0, INET6_ADDRSTRLEN);

        switch (family_)
        {
            case address_family::inter_network:
            {
                in_addr addr;
                memset(&addr, 0, sizeof(in_addr));
                memcpy(&addr, address_.data(), 4);
                inet_ntop(AF_INET, &addr, str, INET_ADDRSTRLEN);
                break;
            }
            case address_family::inter_network_v6:
            {
                in6_addr addr;
                memset(&addr, 0, sizeof(in6_addr));
                memcpy(&addr, address_.data(), 16);
                inet_ntop(AF_INET6, &addr, str, INET6_ADDRSTRLEN);
                break;
            }
        }

        return str;
    }

    address address::parse(const std::string& ip)
    {
        address addr(0);

        if (try_parse(ip, addr))
        {
            return addr;
        }
        else
        {
            throw std::invalid_argument("Given string doesn't map to a valid IP address.");
        }
    }

    bool address::try_parse(const std::string& ip, address& addr)
    {
        std::array<uint8_t, 16> v;

        if (inet_pton(AF_INET, ip.c_str(), v.data()) == 1)
        {
            auto n = *reinterpret_cast<uint32_t*>(v.data());
            addr = address(ntohl(n));
            return true;
        }
        else if (inet_pton(AF_INET6, ip.c_str(), v.data()) == 1)
        {
            addr = address(v);
            return true;
        }

        return false;
    }
}
