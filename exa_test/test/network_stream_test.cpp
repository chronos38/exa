#include <pch.h>
#include <exa/network_stream.hpp>
#include <exa/memory_stream.hpp>
#include <exa/task.hpp>

using namespace exa;
using namespace std::literals::chrono_literals;

TEST(network_stream_test, ctor_null_socket_throws)
{
    ASSERT_THROW(network_stream s(nullptr), std::invalid_argument);
}

TEST(network_stream_test, ctor_not_stream_throws)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::datagram, protocol_type::udp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::datagram, protocol_type::udp);

    listener->bind(address::loopback, 0);
    client->connect_async(address::loopback, listener->local_endpoint().port()).get();
    ASSERT_THROW(network_stream s(client), std::invalid_argument);
}

TEST(network_stream_test, ctor_nonblocking_throws)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    server->blocking(false);
    ASSERT_THROW(network_stream s(server), std::invalid_argument);
}

TEST(network_stream_test, ctor_socket_can_read_and_write_doesnt_own)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    for (int i = 0; i < 2; ++i)
    {
        auto server_stream = std::make_unique<network_stream>(server);
        auto client_stream = std::make_unique<network_stream>(client);

        ASSERT_TRUE(server_stream->can_write() && client_stream->can_read());
        ASSERT_TRUE(client_stream->can_write() && server_stream->can_read());
        ASSERT_FALSE(client_stream->can_seek() && server_stream->can_seek());
        ASSERT_TRUE(client_stream->can_timeout() && server_stream->can_timeout());

        std::vector<uint8_t> buffer(1);

        server_stream->write_async(std::vector<uint8_t>({'a'})).get();
        ASSERT_EQ(1, client_stream->read_async(buffer).get());
        ASSERT_EQ('a', static_cast<char>(buffer[0]));

        client_stream->write_async(std::vector<uint8_t>({'b'})).get();
        ASSERT_EQ(1, server_stream->read_async(buffer).get());
        ASSERT_EQ('b', static_cast<char>(buffer[0]));
    }
}

TEST(network_stream_test, ctor_socket_file_access_bool_can_read_and_write_doesnt_own)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    for (int i = 0; i < 2; ++i)
    {
        auto server_stream = std::make_unique<network_stream>(server, file_access::read_write);
        auto client_stream = std::make_unique<network_stream>(client, file_access::read_write);

        ASSERT_TRUE(server_stream->can_write() && client_stream->can_read());
        ASSERT_TRUE(client_stream->can_write() && server_stream->can_read());
        ASSERT_FALSE(client_stream->can_seek() && server_stream->can_seek());
        ASSERT_TRUE(client_stream->can_timeout() && server_stream->can_timeout());

        std::vector<uint8_t> buffer(1);

        server_stream->write_async(std::vector<uint8_t>({'a'})).get();
        ASSERT_EQ(1, client_stream->read_async(buffer).get());
        ASSERT_EQ('a', static_cast<char>(buffer[0]));

        client_stream->write_async(std::vector<uint8_t>({'b'})).get();
        ASSERT_EQ(1, server_stream->read_async(buffer).get());
        ASSERT_EQ('b', static_cast<char>(buffer[0]));
    }
}

TEST(network_stream_test, ctor_socket_bool_can_read_and_write)
{
    for (auto owns_socket : {true, false})
    {
        auto listener =
            std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
        auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

        listener->bind(address::loopback, 0);
        listener->listen(1);

        auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
        auto server = listener->accept_async().get();
        f.get();

        for (int i = 0; i < 2; ++i)
        {
            try
            {
                auto server_stream = std::make_unique<network_stream>(server, owns_socket);
                auto client_stream = std::make_unique<network_stream>(client, owns_socket);

                ASSERT_TRUE(server_stream->can_write() && client_stream->can_read());
                ASSERT_TRUE(client_stream->can_write() && server_stream->can_read());
                ASSERT_FALSE(client_stream->can_seek() && server_stream->can_seek());
                ASSERT_TRUE(client_stream->can_timeout() && server_stream->can_timeout());

                std::vector<uint8_t> buffer(1);

                server_stream->write_async(std::vector<uint8_t>({'a'})).get();
                ASSERT_EQ(1, client_stream->read_async(buffer).get());
                ASSERT_EQ('a', static_cast<char>(buffer[0]));

                client_stream->write_async(std::vector<uint8_t>({'b'})).get();
                ASSERT_EQ(1, server_stream->read_async(buffer).get());
                ASSERT_EQ('b', static_cast<char>(buffer[0]));
            }
            catch (std::runtime_error&)
            {
                ASSERT_NE(0, i);
                ASSERT_TRUE(owns_socket);
            }
        }
    }
}

TEST(network_stream_test, ctor_socket_file_access_can_read_and_write)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    for (int i = 0; i < 2; ++i)
    {
        auto server_stream = std::make_unique<network_stream>(server, file_access::write);
        auto client_stream = std::make_unique<network_stream>(client, file_access::read);

        ASSERT_TRUE(server_stream->can_write() && !server_stream->can_read());
        ASSERT_TRUE(!client_stream->can_write() && client_stream->can_read());
        ASSERT_FALSE(client_stream->can_seek() && server_stream->can_seek());
        ASSERT_TRUE(client_stream->can_timeout() && server_stream->can_timeout());

        std::vector<uint8_t> buffer(1);

        server_stream->write_async(std::vector<uint8_t>({'a'})).get();
        ASSERT_EQ(1, client_stream->read_async(buffer).get());
        ASSERT_EQ('a', static_cast<char>(buffer[0]));

        ASSERT_THROW(server_stream->read_async(buffer).get(), std::runtime_error);
        ASSERT_THROW(client_stream->write_async(std::vector<uint8_t>({'b'})).get(), std::runtime_error);
    }
}

TEST(network_stream_test, data_available_returns_false_or_true_appropriately)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    auto server_stream = std::make_unique<network_stream>(server);
    auto client_stream = std::make_unique<network_stream>(client);

    ASSERT_FALSE(server_stream->data_available() || client_stream->data_available());
    server_stream->write_async(std::vector<uint8_t>({1})).get();
    ASSERT_FALSE(server_stream->data_available());

    bool b = false;

    for (auto start = std::chrono::system_clock::now(); std::chrono::system_clock::now() - start < 10s;)
    {
        b = client_stream->data_available();

        if (b)
        {
            break;
        }
    }

    ASSERT_TRUE(b);
}

TEST(network_stream_test, close_members_throw)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    auto stream = std::make_unique<network_stream>(server);

    for (int i = 0; i < 2; ++i)
    {
        stream->close();
    }

    std::vector<uint8_t> buffer(1, 1);

    ASSERT_THROW(stream->data_available(), std::runtime_error);
    ASSERT_THROW(stream->read(buffer), std::runtime_error);
    ASSERT_THROW(stream->write(buffer), std::runtime_error);
    ASSERT_THROW(stream->copy_to(std::make_shared<memory_stream>()), std::runtime_error);
    ASSERT_THROW(stream->read_async(buffer), std::runtime_error);
    ASSERT_THROW(stream->write_async(buffer), std::runtime_error);
    ASSERT_THROW(stream->copy_to_async(std::make_shared<memory_stream>()), std::runtime_error);
}

TEST(network_stream_test, read_write_invalid_arguments_throws)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    auto stream = std::make_unique<network_stream>(server);
    gsl::span<uint8_t> buffer;

    ASSERT_THROW(stream->read(buffer), std::invalid_argument);
    ASSERT_THROW(stream->write(buffer), std::invalid_argument);
    ASSERT_THROW(stream->copy_to(nullptr), std::invalid_argument);
    ASSERT_THROW(stream->copy_to(std::make_shared<memory_stream>(), 0), std::out_of_range);
    ASSERT_THROW(stream->copy_to(std::make_shared<memory_stream>(), -1), std::out_of_range);
    ASSERT_THROW(stream->read_async(buffer), std::invalid_argument);
    ASSERT_THROW(stream->write_async(buffer), std::invalid_argument);
    ASSERT_THROW(stream->copy_to_async(std::make_shared<memory_stream>(), 0), std::out_of_range);
    ASSERT_THROW(stream->copy_to_async(std::make_shared<memory_stream>(), -1), std::out_of_range);
}

TEST(network_stream_test, readable_writeable_properties_roundtrip)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    auto stream = std::make_unique<network_stream>(server);
    std::vector<uint8_t> buffer(1, 1);

    stream->readable(false);
    ASSERT_FALSE(stream->readable());
    ASSERT_FALSE(stream->can_read());
    ASSERT_THROW(stream->read(buffer), std::runtime_error);

    stream->readable(true);
    ASSERT_TRUE(stream->readable());
    ASSERT_TRUE(stream->can_read());
    auto f2 = client->send_async(buffer);
    ASSERT_EQ(1, stream->read_async(buffer).get());
    f2.get();

    stream->writable(false);
    ASSERT_FALSE(stream->writable());
    ASSERT_FALSE(stream->can_write());
    ASSERT_THROW(stream->write(buffer), std::runtime_error);

    stream->writable(true);
    ASSERT_TRUE(stream->writable());
    ASSERT_TRUE(stream->can_write());
    auto f3 = stream->write_async(buffer);
    ASSERT_EQ(1, client->receive_async(buffer).get());
    f3.get();
}

TEST(network_stream_test, read_write_byte_success)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    auto server_stream = std::make_shared<network_stream>(server);
    auto client_stream = std::make_shared<network_stream>(client);

    for (uint8_t i = 0; i < 10; ++i)
    {
        auto read = task::run([=] { return client_stream->read_byte(); });
        task::run([=] { server_stream->write_byte(i); }).get();
        ASSERT_EQ(i, read.get());
    }
}

TEST(network_stream_test, read_write_array_success)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    auto server_stream = std::make_shared<network_stream>(server);
    auto client_stream = std::make_shared<network_stream>(client);

    std::vector<uint8_t> client_data(42, 1);
    client_stream->write(client_data);

    std::vector<uint8_t> server_data(42, 0);
    server_stream->read(server_data);

    for (size_t i = 0; i < 42; ++i)
    {
        ASSERT_EQ(client_data[i], server_data[i]);
    }
}

TEST(network_stream_test, read_timeout_expires_throws)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    auto server_stream = std::make_shared<network_stream>(server);
    std::vector<uint8_t> buffer(1, 1);

    server_stream->read_timeout(1ms);
    ASSERT_THROW(server_stream->read(buffer), std::system_error);
}

TEST(network_stream_test, timeout_valid_data_roundtrip)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.get();

    auto server_stream = std::make_shared<network_stream>(server);

    server_stream->read_timeout(100ms);
    ASSERT_EQ(100ms, server_stream->read_timeout());
    server_stream->read_timeout(100ms);
    ASSERT_EQ(100ms, server_stream->read_timeout());

    server_stream->read_timeout(-1ms);
    ASSERT_EQ(-1ms, server_stream->read_timeout());

    server_stream->write_timeout(100ms);
    ASSERT_EQ(100ms, server_stream->write_timeout());
    server_stream->write_timeout(100ms);
    ASSERT_EQ(100ms, server_stream->write_timeout());

    server_stream->write_timeout(-1ms);
    ASSERT_EQ(-1ms, server_stream->write_timeout());
}

TEST(network_stream_test, copy_to_async_all_data_copied)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, std::numeric_limits<uint8_t>::max());

    for (auto count : {1, 1024, 4096, 4095, 1024 * 1024})
    {
        auto listener =
            std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
        auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

        listener->bind(address::loopback, 0);
        listener->listen(1);

        auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
        auto server = listener->accept_async().get();
        f.wait();

        auto server_stream = std::make_shared<network_stream>(server);
        auto client_stream = std::make_shared<network_stream>(client);

        auto results = std::make_shared<memory_stream>();
        std::vector<uint8_t> data(count);

        for (auto& b : data)
        {
            b = dis(gen);
        }

        auto copy = client_stream->copy_to_async(results);
        server_stream->write_async(data).wait();
        server_stream->close();
        copy.wait();

        auto result = results->to_array();
        ASSERT_EQ(data.size(), result.size());

        for (size_t i = 0; i < data.size(); ++i)
        {
            ASSERT_EQ(data[i], result[i]);
        }
    }
}

TEST(network_stream_test, copy_to_async_invalid_arguments_throw)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.wait();

    auto server_stream = std::make_shared<network_stream>(server);
    auto client_stream = std::make_shared<network_stream>(client);
    auto ms = std::make_shared<memory_stream>(std::vector<uint8_t>());

    ASSERT_THROW(server_stream->copy_to_async(nullptr).get(), std::invalid_argument);
    client_stream->write_byte(42);
    ASSERT_THROW(server_stream->copy_to_async(ms).get(), std::runtime_error);
    ASSERT_THROW(server_stream->copy_to_async(std::make_shared<memory_stream>(), 0).get(), std::out_of_range);
    ASSERT_THROW(server_stream->copy_to_async(std::make_shared<memory_stream>(), -1).get(), std::out_of_range);
}

TEST(network_stream_test, copy_to_async_close_stream_throws_on_windows_no_throw_on_unix)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.wait();

    auto server_stream = std::make_shared<network_stream>(server);
    auto client_stream = std::make_shared<network_stream>(client);
    auto copy = server_stream->copy_to_async(std::make_shared<memory_stream>());
    server_stream->close();

#ifdef _WIN32
    ASSERT_THROW(copy.get(), std::runtime_error);
#endif
    ASSERT_THROW(server_stream->copy_to_async(std::make_shared<memory_stream>()).get(), std::runtime_error);
}

TEST(network_stream_test, copy_to_async_non_readable_source_stream_throws)
{
    auto listener = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);
    auto client = std::make_shared<exa::socket>(address_family::inter_network, socket_type::stream, protocol_type::tcp);

    listener->bind(address::loopback, 0);
    listener->listen(1);

    auto f = client->connect_async(address::loopback, listener->local_endpoint().port());
    auto server = listener->accept_async().get();
    f.wait();

    auto server_stream = std::make_shared<network_stream>(server, file_access::write);

    ASSERT_THROW(server_stream->copy_to_async(std::make_shared<memory_stream>()).get(), std::runtime_error);
}
