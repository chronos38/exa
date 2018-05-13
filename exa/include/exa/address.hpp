#pragma once

#include <exa/socket_base.hpp>
#include <exa/dependencies.hpp>

#include <string>
#include <cstdint>
#include <vector>
#include <cstddef>

namespace exa
{
    class address
    {
    public:
        address();
        explicit address(uint32_t ipv4);
        explicit address(gsl::span<const uint8_t> ipv6, uint32_t scope_id = 0);
        explicit address(std::initializer_list<const uint8_t> ipv6, uint32_t scope_id = 0);

        const std::vector<uint8_t>& bytes() const;
        address_family family() const;
        uint32_t scope_id() const;
        void scope_id(uint32_t value);

        std::string to_string() const;

        static address parse(const std::string& ip);
        static bool try_parse(const std::string& ip, address& addr);

        static const address any;
        static const address loopback;
        static const address broadcast;
        static const address ipv6_any;
        static const address ipv6_loopback;
        static const address ipv6_broadcast;

    private:
        address_family family_;
        std::vector<uint8_t> address_;
        uint32_t scope_id_ = 0;
    };
}
