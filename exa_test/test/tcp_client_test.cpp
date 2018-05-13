#include <pch.h>
#include <exa/tcp_client.hpp>
#include <exa/tcp_listener.hpp>

using namespace exa;
using namespace std::literals::chrono_literals;

TEST(tcp_client_test, ctor_null_socket_throws)
{
    ASSERT_THROW(tcp_client c(nullptr), std::invalid_argument);
}

TEST(tcp_client_test, ctor_invalid_family_throws)
{
    ASSERT_THROW(tcp_client c(address_family::apple_talk), std::invalid_argument);
    ASSERT_THROW(tcp_client c(address_family::ipx), std::invalid_argument);
    ASSERT_THROW(tcp_client c(address_family::unix_descriptor), std::invalid_argument);
    ASSERT_THROW(tcp_client c(address_family::unspecified), std::invalid_argument);
}

TEST(tcp_client_test, get_stream_not_connected_throws)
{
    tcp_client c;
    ASSERT_THROW(c.stream(), std::runtime_error);
}

TEST(tcp_client_test, close_operations_throw)
{
    tcp_client c;

    for (int i = 0; i < 2; ++i)
    {
        c.close();
    }

    ASSERT_THROW(c.connect(address::loopback, 0), std::runtime_error);
    ASSERT_THROW(c.connect("localhost", 0), std::runtime_error);
    ASSERT_THROW(c.stream(), std::runtime_error);
}

TEST(tcp_client_test, ctor_string_int_connects_successfully)
{
    auto l = tcp_listener::create(0);
    l->start();

    tcp_client c("localhost", l->local_endpoint().port());
    ASSERT_TRUE(c.connected());
}

TEST(tcp_client_test, connect_async_dns_end_point_success)
{
    auto l = tcp_listener::create(0);
    l->start();
    auto port = l->local_endpoint().port();

    for (auto mode : {0, 1})
    {
        tcp_client c;
        ASSERT_FALSE(c.connected());

        switch (mode)
        {
            case 0:
                c.connect_async("localhost", port).get();
                break;
            case 1:
            {
                auto endpoints = endpoint::get_address_info("localhost", std::to_string(port));
                EXPECT_FALSE(endpoints.empty());
                c.connect_async(endpoints).get();
                break;
            }
        }

        auto peer = l->accept_client();
        ASSERT_TRUE(c.connected());

        auto s = c.stream();
        std::stringstream stream;
        stream << "GET / HTTP/1.1\r\n\r\n";
        auto str = stream.str();
        auto request = reinterpret_cast<uint8_t*>(str.data());
        s->write_async(gsl::span<uint8_t>(request, str.size())).get();
        ASSERT_NE(-1, peer->stream()->read_byte());
    }
}

TEST(tcp_client_test, connected_available_initial_values_default)
{
    tcp_client c;
    ASSERT_FALSE(c.connected());
    ASSERT_EQ(0, c.available());
}

#ifdef _WIN32
TEST(tcp_client_test, roundtrip_exclusive_address_use_get_equals_set_true)
{
    tcp_client c;
    c.exclusive_address_use(true);
    ASSERT_TRUE(c.exclusive_address_use());
}

TEST(tcp_client_test, roundtrip_exclusive_address_use_get_equals_set_false)
{
    tcp_client c;
    c.exclusive_address_use(false);
    ASSERT_FALSE(c.exclusive_address_use());
}
#endif

TEST(tcp_client_test, roundtrip_linger_option_get_equals_set)
{
    tcp_client c;

    c.linger_state(linger_option{true, 42s});
    ASSERT_TRUE(c.linger_state().enabled);
    ASSERT_EQ(c.linger_state().linger_time, 42s);

    c.linger_state(linger_option{true, 0s});
    ASSERT_TRUE(c.linger_state().enabled);
    ASSERT_EQ(c.linger_state().linger_time, 0s);

    c.linger_state(linger_option{false, 0s});
    ASSERT_FALSE(c.linger_state().enabled);
    ASSERT_EQ(c.linger_state().linger_time, 0s);
}

TEST(tcp_client_test, roundtrip_no_delay_get_equals_set)
{
    tcp_client c;

    c.no_delay(true);
    ASSERT_TRUE(c.no_delay());

    c.no_delay(false);
    ASSERT_FALSE(c.no_delay());
}

TEST(tcp_client_test, roundtrip_receive_buffer_size_get_equals_set)
{
    tcp_client c;

    c.receive_buffer(4096);
    ASSERT_LE(4096, c.receive_buffer());

    c.receive_buffer(8192);
    ASSERT_LE(8192, c.receive_buffer());
}

TEST(tcp_client_test, roundtrip_send_buffer_size_get_equals_set)
{
    tcp_client c;

    c.send_buffer(4096);
    ASSERT_LE(4096, c.send_buffer());

    c.send_buffer(8192);
    ASSERT_LE(8192, c.send_buffer());
}

TEST(tcp_client_test, roundtrip_receive_timeout_get_equals_set)
{
    tcp_client c;

    c.receive_timeout(1ms);
    ASSERT_LE(1ms, c.receive_timeout());

    c.receive_timeout(0ms);
    ASSERT_EQ(0ms, c.receive_timeout());
}

TEST(tcp_client_test, roundtrip_send_timeout_get_equals_set)
{
    tcp_client c;

    c.send_timeout(1ms);
    ASSERT_LE(1ms, c.send_timeout());

    c.send_timeout(0ms);
    ASSERT_EQ(0ms, c.send_timeout());
}

TEST(tcp_client_test, properties_persist_after_connect)
{
    auto l = tcp_listener::create(0);
    l->start();
    auto port = l->local_endpoint().port();
    tcp_client c;

    c.linger_state(linger_option{true, 1s});
    c.receive_timeout(42ms);
    c.send_timeout(84ms);

    c.connect_async("localhost", port).get();

    ASSERT_TRUE(c.linger_state().enabled);
    ASSERT_EQ(1s, c.linger_state().linger_time);
    ASSERT_LE(42ms, c.receive_timeout());
    ASSERT_LE(84ms, c.send_timeout());
}

TEST(tcp_client_test, close_cancels_connect_async)
{
    for (auto b : {true, false})
    {
        exa::socket s(address_family::inter_network, socket_type::stream, protocol_type::tcp);
        s.bind(endpoint(address::loopback, 0));
        s.listen(1);
        auto ep = s.local_endpoint();

        tcp_client c;
        auto f = b ? c.connect_async("localhost", ep.port()) : c.connect_async(ep.address(), ep.port());
        c.close();

        ASSERT_ANY_THROW(f.get());
        ASSERT_FALSE(c.connected());
    }
}
