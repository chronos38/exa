#include <pch.h>
#include <exa/udp_client.hpp>

using namespace exa;
using namespace testing;
using namespace std::chrono_literals;

namespace
{
    constexpr auto unused_port = 1895;
    constexpr auto udp_redundancy = 1;
}

TEST(udp_client_test, ctor_valid_address_family_succeeds)
{
    ASSERT_NO_THROW(udp_client c(address_family::inter_network));
    ASSERT_NO_THROW(udp_client c(address_family::inter_network_v6));
}

TEST(udp_client_test, ctor_invalid_address_family_throws)
{
    ASSERT_THROW(udp_client c(address_family::unspecified), std::invalid_argument);
    ASSERT_THROW(udp_client c(address_family::apple_talk), std::invalid_argument);
    ASSERT_THROW(udp_client c(address_family::unix_descriptor), std::invalid_argument);
}

TEST(udp_client_test, DISABLED_ctor_invalid_host_name_throws)
{
    ASSERT_THROW(udp_client c("foo", 0), std::runtime_error);
}

TEST(udp_client_test, ctor_can_send)
{
    udp_client c;
    ASSERT_THAT(c.send(std::vector<uint8_t>({1}), endpoint(address::loopback, unused_port)), Eq(1));
    ASSERT_FALSE(c.connected());
}

TEST(udp_client_test, ctor_int_can_send)
{
    try
    {
        udp_client c(unused_port);
        ASSERT_THAT(c.send(std::vector<uint8_t>({1}), endpoint(address::loopback, unused_port)), Eq(1));
    }
    catch (std::system_error& e)
    {
#ifdef _WIN32
        ASSERT_THAT(e.code().value(), Eq(WSAEACCES));
#else
        ASSERT_THAT(e.code().value(), Eq(EACCES));
#endif
    }
}

TEST(udp_client_test, ctor_int_address_family_ipv4_can_send)
{
    try
    {
        udp_client c(unused_port, address_family::inter_network);
        ASSERT_THAT(c.send(std::vector<uint8_t>({1}), endpoint(address::loopback, unused_port)), Eq(1));
    }
    catch (std::system_error& e)
    {
#ifdef _WIN32
        ASSERT_THAT(e.code().value(), Eq(WSAEACCES));
#else
        ASSERT_THAT(e.code().value(), Eq(EACCES));
#endif
    }
}

TEST(udp_client_test, ctor_int_address_family_ipv6_can_send)
{
    try
    {
        udp_client c(unused_port, address_family::inter_network_v6);
        ASSERT_THAT(c.send(std::vector<uint8_t>({1}), endpoint(address::ipv6_loopback, unused_port)), Eq(1));
    }
    catch (std::system_error& e)
    {
#ifdef _WIN32
        ASSERT_THAT(e.code().value(), Eq(WSAEACCES));
#else
        ASSERT_THAT(e.code().value(), Eq(EACCES));
#endif
    }
}

TEST(udp_client_test, ctor_endpoint_can_send)
{
    try
    {
        udp_client c(endpoint(address::ipv6_any, unused_port));
        ASSERT_EQ(1, c.send(std::vector<uint8_t>({1}), endpoint(address::ipv6_loopback, unused_port)));
    }
    catch (std::system_error& e)
    {
#ifdef _WIN32
        ASSERT_THAT(e.code().value(), Eq(WSAEACCES));
#else
        ASSERT_THAT(e.code().value(), Eq(EACCES));
#endif
    }
}

TEST(udp_client_test, ctor_string_int_can_send)
{
    try
    {
        udp_client c("localhost", unused_port);
        ASSERT_EQ(1, c.send(std::vector<uint8_t>({1})));
    }
    catch (std::system_error& e)
    {
#ifdef _WIN32
        ASSERT_THAT(e.code().value(), Eq(WSAEACCES));
#else
        ASSERT_THAT(e.code().value(), Eq(EACCES));
#endif
    }
}

TEST(udp_client_test, closed_client_throws)
{
    udp_client c;

    for (int i = 0; i < 2; ++i)
    {
        c.close();
    }

    endpoint ep;

    ASSERT_THROW(c.connect(endpoint{}), std::runtime_error);
    ASSERT_THROW(c.send({}, endpoint{}), std::runtime_error);
    ASSERT_THROW(c.receive(ep), std::runtime_error);
}

TEST(udp_client_test, ttl_roundtrips)
{
    udp_client c;
    auto ttl = c.ttl();
    ASSERT_THAT(c.ttl(), Eq(ttl));

    c.ttl(100s);
    ASSERT_THAT(c.ttl(), Eq(100s));
}

TEST(udp_client_test, enable_broadcast_roundtrips)
{
    udp_client c;
    ASSERT_FALSE(c.enable_broadcast());

    c.enable_broadcast(true);
    ASSERT_TRUE(c.enable_broadcast());

    c.enable_broadcast(false);
    ASSERT_FALSE(c.enable_broadcast());
}

TEST(udp_client_test, send_invalid_arguments_throw)
{
    udp_client c;

    ASSERT_THROW(c.send({}), std::invalid_argument);
    ASSERT_THROW(c.send({}, endpoint{}), std::invalid_argument);
    ASSERT_THROW(c.send({}, endpoint(address::loopback, 0)), std::invalid_argument);
    ASSERT_THROW(c.send(std::vector<uint8_t>({1})), std::runtime_error);
}

#ifdef _WIN32
TEST(udp_client_test, send_invalid_arguments_string_int_throws)
{
    udp_client c("localhost", 0);
    ASSERT_THROW(c.send(std::vector<uint8_t>({1}), endpoint(address::loopback, 0)), std::runtime_error);
}
#endif

TEST(udp_client_test, connect_invalid_arguments_windows_throws_unix_no_throw)
{
    udp_client c;
#ifdef _WIN32
    ASSERT_THROW(c.connect(endpoint{}), std::runtime_error);
#else
    ASSERT_NO_THROW(c.connect(endpoint{}));
#endif
}

TEST(udp_client_test, connect_async_address_host_success)
{
    udp_client c;
    auto f = c.socket()->connect_async(endpoint(address::parse("114.114.114.114"), 53));
    ASSERT_NO_THROW(f.get());
}

TEST(udp_client_test, connect_string_host_success)
{
    udp_client c;
    ASSERT_NO_THROW(c.socket()->connect(endpoint(address::parse("114.114.114.114"), 53)));
}

TEST(udp_client_test, send_receive_success)
{
    for (auto ipv4 : {false, true})
    {
        auto address = ipv4 ? address::loopback : address::ipv6_loopback;
        udp_client receiver(endpoint(address, 0));
        udp_client sender(endpoint(address, 0));

        for (int i = 0; i < udp_redundancy; ++i)
        {
            sender.send(std::vector<uint8_t>({1}), endpoint(address, receiver.socket()->local_endpoint().port()));
        }

        endpoint ep;
        auto data = receiver.receive(ep);

        ASSERT_THAT(ep.port(), Eq(sender.socket()->local_endpoint().port()));
        ASSERT_THAT(data.size(), Ne(0));
    }
}

TEST(udp_client_test, send_receive_connected_success)
{
    udp_client receiver("localhost", 0);
    udp_client sender("localhost", receiver.socket()->local_endpoint().port());

    for (int i = 0; i < udp_redundancy; ++i)
    {
        sender.send(std::vector<uint8_t>({1}));
    }

    endpoint ep;
    auto data = receiver.receive(ep);

    ASSERT_THAT(ep.port(), Eq(sender.socket()->local_endpoint().port()));
    ASSERT_THAT(data.size(), Ne(0));
}

TEST(udp_client_test, send_available_success)
{
    for (auto ipv4 : {false, true})
    {
        auto address = ipv4 ? address::loopback : address::ipv6_loopback;
        udp_client receiver(endpoint(address, 0));
        udp_client sender(endpoint(address, 0));

        for (int i = 0; i < udp_redundancy; ++i)
        {
            sender.send(std::vector<uint8_t>({1}), endpoint(address, receiver.socket()->local_endpoint().port()));
        }

        size_t n = 0;

        for (auto start = std::chrono::system_clock::now(); std::chrono::system_clock::now() - start < 30s;)
        {
            n = receiver.available();

            if (n > 0)
            {
                break;
            }
        }

        ASSERT_THAT(n, Ne(0));
    }
}

TEST(udp_client_test, send_async_receive_async_success)
{
    for (auto ipv4 : {false, true})
    {
        auto address = ipv4 ? address::loopback : address::ipv6_loopback;
        udp_client receiver(endpoint(address, 0));
        udp_client sender(endpoint(address, 0));

        for (int i = 0; i < udp_redundancy; ++i)
        {
            sender.send_async(std::vector<uint8_t>({1}), endpoint(address, receiver.socket()->local_endpoint().port()))
                .get();
        }

        auto result = receiver.receive_async().get();
        ASSERT_THAT(result.endpoint.port(), Eq(sender.socket()->local_endpoint().port()));
        ASSERT_THAT(result.buffer.size(), Ne(0));
    }
}

TEST(udp_client_test, send_async_receive_async_connected_success)
{
    udp_client receiver("localhost", 0);
    udp_client sender("localhost", receiver.socket()->local_endpoint().port());

    for (int i = 0; i < udp_redundancy; ++i)
    {
        sender.send_async(std::vector<uint8_t>({1})).get();
    }

    auto result = receiver.receive_async().get();
    ASSERT_THAT(result.endpoint.port(), Eq(sender.socket()->local_endpoint().port()));
    ASSERT_THAT(result.buffer.size(), Ne(0));
}
