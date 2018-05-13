#pragma once

#include <exa/address.hpp>
#include <exa/socket_base.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <cstddef>

namespace exa
{
    class endpoint
    {
    public:
        endpoint();
        endpoint(uint32_t address, uint16_t port);
        endpoint(const address& address, uint16_t port);
        explicit endpoint(const sockaddr_storage& storage);

        address_family family() const;
        const address& address() const;
        uint16_t port() const;
        std::vector<uint8_t> serialize() const;

        static std::vector<endpoint> get_address_info(const std::string& host, const std::string& service);

    private:
        exa::address address_;
        uint16_t port_ = 0;
    };
}
