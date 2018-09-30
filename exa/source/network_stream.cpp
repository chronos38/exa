#include <exa/network_stream.hpp>
#include <exa/enum_flag.hpp>
#include <exa/task.hpp>

#include <limits>

using namespace std::chrono_literals;

namespace exa
{
    network_stream::network_stream(const std::shared_ptr<exa::socket>& socket, bool owns)
        : network_stream(socket, file_access::read_write, owns)
    {
    }

    network_stream::network_stream(const std::shared_ptr<exa::socket>& s, file_access access, bool owns)
        : socket_(s), owns_(owns)
    {
        if (s == nullptr)
        {
            throw std::invalid_argument("Socket for network stream is nullptr.");
        }
        if (s->type() != socket_type::stream)
        {
            throw std::invalid_argument("Network stream works only for socket of type stream.");
        }
        if (!s->connected())
        {
            throw std::invalid_argument("Network stream works only for connected sockets.");
        }
        if (!s->blocking())
        {
            throw std::invalid_argument("Network stream needs to be in blocking mode.");
        }

        readable_ = has_flag(access, file_access::read);
        writable_ = has_flag(access, file_access::write);
    }

    network_stream::~network_stream()
    {
        if (owns_)
        {
            close();
        }
    }

    bool network_stream::can_read() const
    {
        return readable_;
    }

    bool network_stream::can_seek() const
    {
        return false;
    }

    bool network_stream::can_timeout() const
    {
        return true;
    }

    bool network_stream::can_write() const
    {
        return writable_;
    }

    std::streamsize network_stream::size() const
    {
        throw std::runtime_error("Getting size of network stream isn't supported.");
    }

    void network_stream::size(std::streamoff)
    {
        throw std::runtime_error("Set length of network stream isn't supported.");
    }

    std::streamoff network_stream::position() const
    {
        throw std::runtime_error("Getting position of network stream isn't supported.");
    }

    void network_stream::position(std::streamoff)
    {
        throw std::runtime_error("Setting position of network stream isn't supported.");
    }

    std::chrono::milliseconds network_stream::read_timeout() const
    {
        return socket_->receive_timeout();
    }

    void network_stream::read_timeout(const std::chrono::milliseconds& value)
    {
        socket_->receive_timeout(value);
    }

    std::chrono::milliseconds network_stream::write_timeout() const
    {
        return socket_->send_timeout();
    }

    void network_stream::write_timeout(const std::chrono::milliseconds& value)
    {
        socket_->send_timeout(value);
    }

    bool network_stream::readable() const
    {
        return readable_;
    }

    void network_stream::readable(bool value)
    {
        readable_ = value;
    }

    bool network_stream::writable() const
    {
        return writable_;
    }

    void network_stream::writable(bool value)
    {
        writable_ = value;
    }

    void network_stream::close()
    {
        socket_->close();
    }

    std::future<void> network_stream::copy_to_async(std::shared_ptr<stream> s, std::streamsize buffer_size)
    {
        if (!socket_->valid())
        {
            throw std::runtime_error("Invalid socket in network stream.");
        }

        return stream::copy_to_async(s, buffer_size);
    }

    void network_stream::flush()
    {
    }

    std::future<void> network_stream::flush_async()
    {
        return std::async(std::launch::deferred, [] {});
    }

    std::streamsize network_stream::read(gsl::span<uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Read buffer is a nullptr.");
        }
        if (!readable_)
        {
            throw std::runtime_error("Reading isn't supported for this network stream.");
        }

        return static_cast<size_t>(socket_->receive(buffer));
    }

    std::future<std::streamsize> network_stream::read_async(gsl::span<uint8_t> buffer)
    {
        if (!socket_->valid())
        {
            throw std::runtime_error("Invalid socket in network stream.");
        }

        return stream::read_async(buffer);
    }

    std::streamoff network_stream::seek(std::streamoff, seek_origin)
    {
        throw std::runtime_error("Seeking of network stream isn't supported.");
    }

    void network_stream::write(gsl::span<const uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Write buffer is a nullptr.");
        }
        if (!writable_)
        {
            throw std::runtime_error("Writing isn't supported for this network stream.");
        }

        auto n = socket_->send(buffer);

        if (n != static_cast<size_t>(buffer.size()))
        {
            throw std::runtime_error("Not all bytes were written to the network stream.");
        }
    }

    std::future<void> network_stream::write_async(gsl::span<const uint8_t> buffer)
    {
        if (!socket_->valid())
        {
            throw std::runtime_error("Invalid socket in network stream.");
        }

        return stream::write_async(buffer);
    }

    bool network_stream::data_available() const
    {
        return socket_->available() > 0;
    }

    const std::shared_ptr<socket>& network_stream::socket() const
    {
        return socket_;
    }
}
