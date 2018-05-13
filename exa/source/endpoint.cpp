#include <exa/endpoint.hpp>

namespace exa
{
    endpoint::endpoint()
    {
    }

    endpoint::endpoint(uint32_t address, uint16_t port) : endpoint(exa::address(address), port)
    {
    }

    endpoint::endpoint(const exa::address& address, uint16_t port) : address_(address), port_(port)
    {
    }

    endpoint::endpoint(const sockaddr_storage& storage)
    {
        switch (storage.ss_family)
        {
            case AF_INET:
            {
                auto addr = reinterpret_cast<const sockaddr_in*>(&storage);
                auto int_addr = reinterpret_cast<const uint32_t*>(&addr->sin_addr);
                address_ = exa::address(ntohl(*int_addr));
                port_ = ntohs(addr->sin_port);
                break;
            }
            case AF_INET6:
            {
                auto addr = reinterpret_cast<const sockaddr_in6*>(&storage);
                auto byte_addr = reinterpret_cast<const uint8_t*>(&addr->sin6_addr);
                address_ = exa::address(gsl::span<const uint8_t>(byte_addr, sizeof(in6_addr)), addr->sin6_scope_id);
                port_ = ntohs(addr->sin6_port);
                break;
            }
            default:
                break;
        }
    }

    address_family endpoint::family() const
    {
        return address_.family();
    }

    const address& endpoint::address() const
    {
        return address_;
    }

    uint16_t endpoint::port() const
    {
        return port_;
    }

    std::vector<uint8_t> endpoint::serialize() const
    {
        switch (address_.family())
        {
            case address_family::inter_network:
            {
                sockaddr_in storage = {0};
                auto addr = &storage;
                auto bytes = address_.bytes();
                auto n = std::min<size_t>(sizeof(addr->sin_addr), bytes.size());
                addr->sin_family = AF_INET;
                addr->sin_port = htons(port_);
                memcpy(&addr->sin_addr, bytes.data(), n);
                auto p = reinterpret_cast<uint8_t*>(&storage);
                return std::vector<uint8_t>(p, p + sizeof(storage));
            }
            case address_family::inter_network_v6:
            {
                sockaddr_in6 storage = {0};
                auto addr = &storage;
                auto bytes = address_.bytes();
                auto n = std::min<size_t>(sizeof(addr->sin6_addr), bytes.size());
                addr->sin6_family = AF_INET6;
                addr->sin6_port = htons(port_);
                addr->sin6_scope_id = htonl(address_.scope_id());
                memcpy(&addr->sin6_addr, bytes.data(), n);
                auto p = reinterpret_cast<uint8_t*>(&storage);
                return std::vector<uint8_t>(p, p + sizeof(storage));
            }
            default:
                throw std::runtime_error("Address family is invalid for serialization.");
        }
    }

    std::vector<endpoint> endpoint::get_address_info(const std::string& host, const std::string& service)
    {
        addrinfo* iterator = nullptr;

        auto rc = getaddrinfo(host.c_str(), service.c_str(), nullptr, &iterator);

        if (rc != 0)
        {
            throw std::system_error(rc, std::system_category(), "getaddrinfo");
        }
        if (iterator == nullptr)
        {
            throw std::runtime_error("Couldn't resolve address.");
        }

        std::vector<endpoint> result;

        for (auto it = iterator; it != nullptr; it = it->ai_next)
        {
            switch (it->ai_family)
            {
                case AF_INET:
                {
                    auto addr = reinterpret_cast<sockaddr_in*>(it->ai_addr);
                    auto int_addr = reinterpret_cast<uint32_t*>(&addr->sin_addr);
                    endpoint ep(ntohl(*int_addr), ntohs(addr->sin_port));
                    result.push_back(std::move(ep));
                    break;
                }
                case AF_INET6:
                {
                    auto addr = reinterpret_cast<sockaddr_in6*>(it->ai_addr);
                    auto buf = reinterpret_cast<uint8_t*>(&addr->sin6_addr);
                    exa::address ip_addr(gsl::span<uint8_t>(buf, sizeof(addr->sin6_addr)));
                    endpoint ep(ip_addr, ntohs(addr->sin6_port));
                    result.push_back(std::move(ep));
                    break;
                }
                default:
                    break;
            }
        }

        return result;
    }
}
