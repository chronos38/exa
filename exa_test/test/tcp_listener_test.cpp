#include <pch.h>
#include <exa/tcp_listener.hpp>
#include <exa/tcp_client.hpp>

using namespace exa;
using namespace testing;
using namespace std::chrono_literals;

TEST(tcp_listener_test, active_true_while_running)
{
    for (auto ctor : {0, 1, 2})
    {
        auto l = ctor == 0
                     ? std::make_unique<tcp_listener>(endpoint(address::loopback, 0))
                     : ctor == 1 ? std::make_unique<tcp_listener>(address::loopback, 0) : std::make_unique<tcp_listener>(0);

        ASSERT_FALSE(l->active());
        l->start();
        ASSERT_TRUE(l->active());
        ASSERT_THROW(l->exclusive_address_use(true), std::runtime_error);
        ASSERT_THROW(l->exclusive_address_use(false), std::runtime_error);
        ASSERT_THAT(l->exclusive_address_use(), Eq(l->exclusive_address_use()));
        ASSERT_THROW(l->reuse_address(true), std::runtime_error);
        ASSERT_THROW(l->reuse_address(false), std::runtime_error);
        ASSERT_THAT(l->reuse_address(), Eq(l->reuse_address()));
        l->stop();
        ASSERT_FALSE(l->active());
    }
}

TEST(tcp_listener_test, start_invalid_args_throws)
{
    tcp_listener l(address::loopback, 0);

    ASSERT_THROW(l.start(tcp_listener::max_connections + 1), std::out_of_range);
    ASSERT_FALSE(l.active());

    ASSERT_NO_THROW(l.start(1));
    ASSERT_NO_THROW(l.start(1));
    ASSERT_NO_THROW(l.stop());
}

TEST(tcp_listener_test, pending_true_when_waiting_request)
{
    tcp_listener l(address::loopback, 0);

    ASSERT_THROW(l.pending(), std::runtime_error);
    l.start();
    ASSERT_FALSE(l.pending());

    tcp_client c(address::loopback, 0);
    auto f = c.connect_async(address::loopback, l.local_endpoint().port());

    bool b = false;

    for (auto start = std::chrono::system_clock::now(); std::chrono::system_clock::now() - start < 30s;)
    {
        b = l.pending();

        if (b)
        {
            break;
        }
    }

    ASSERT_TRUE(b);
    l.accept_socket()->close();
    f.get();
    l.stop();
    ASSERT_THROW(l.pending(), std::runtime_error);
}

TEST(tcp_listener_test, accept_invalid_throws)
{
    tcp_listener l(address::loopback, 0);

    ASSERT_THROW(l.accept_socket(), std::runtime_error);
    ASSERT_THROW(l.accept_client(), std::runtime_error);
    ASSERT_THROW(l.accept_socket_async(), std::runtime_error);
    ASSERT_THROW(l.accept_client_async(), std::runtime_error);
}

TEST(tcp_listener_test, accept_accepts_pending_socket_or_client)
{
    tcp_listener l(address::loopback, 0);
    l.start();

    {
        tcp_client c;
        auto f = c.connect_async(address::loopback, l.local_endpoint().port());
        auto s = l.accept_socket();
        ASSERT_FALSE(l.pending());
    }
    {
        tcp_client c;
        auto f = c.connect_async(address::loopback, l.local_endpoint().port());
        auto s = l.accept_client();
        ASSERT_FALSE(l.pending());
    }

    l.stop();
}

TEST(tcp_listener_test, ipv6_only_works)
{
    if (socket::ipv6_supported() || !socket::ipv4_supported())
    {
        return;
    }

    auto l = tcp_listener::create(0);
    l->stop();

    exa::socket s(socket_type::stream, protocol_type::tcp);
}
