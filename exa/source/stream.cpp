#include <exa/stream.hpp>
#include <exa/task.hpp>

#include <functional>
#include <vector>

using namespace std::literals::chrono_literals;

namespace exa
{
    bool stream::can_read() const
    {
        return false;
    }

    bool stream::can_seek() const
    {
        return false;
    }

    bool stream::can_timeout() const
    {
        return false;
    }

    bool stream::can_write() const
    {
        return false;
    }

    std::streamsize stream::size() const
    {
        return 0;
    }

    std::streamoff stream::position() const
    {
        return 0;
    }

    void stream::position(std::streamoff)
    {
    }

    std::chrono::milliseconds stream::read_timeout() const
    {
        return 0ms;
    }

    void stream::read_timeout(const std::chrono::milliseconds&)
    {
    }

    std::chrono::milliseconds stream::write_timeout() const
    {
        return 0ms;
    }

    void stream::write_timeout(const std::chrono::milliseconds&)
    {
    }

    void stream::close()
    {
    }

    void stream::copy_to(std::shared_ptr<stream> s)
    {
        if (s == nullptr)
        {
            throw std::invalid_argument("Can't copy to nullptr stream.");
        }

        auto buffer_size = default_buffer_size;

        if (can_seek())
        {
            auto len = size();
            auto pos = position();

            if (len <= pos)
            {
                buffer_size = 1;
            }
            else
            {
                auto diff = len - pos;

                if (diff > 0)
                {
                    buffer_size = std::min<std::streamsize>(buffer_size, diff);
                }
            }
        }

        copy_to(s, buffer_size);
    }

    void stream::copy_to(std::shared_ptr<stream> s, std::streamsize buffer_size)
    {
        if (s == nullptr)
        {
            throw std::invalid_argument("Can't copy to nullptr stream.");
        }
        if (buffer_size <= 0)
        {
            throw std::out_of_range("Can't copy to a stream with buffer size lower than or equal to 0.");
        }

        std::vector<uint8_t> v(static_cast<size_t>(buffer_size));
        auto r = read(v);

        while (r > 0)
        {
            v.resize(static_cast<size_t>(r));
            s->write(gsl::span<uint8_t>(v.data(), static_cast<std::ptrdiff_t>(r)));
            v.resize(static_cast<size_t>(buffer_size));
            r = read(v);
        }
    }

    std::future<void> stream::copy_to_async(std::shared_ptr<stream> s)
    {
        if (s == nullptr)
        {
            throw std::invalid_argument("Can't copy to nullptr stream.");
        }

        return task::run([=] { return copy_to(s); });
    }

    std::future<void> stream::copy_to_async(std::shared_ptr<stream> s, std::streamsize buffer_size)
    {
        if (s == nullptr)
        {
            throw std::invalid_argument("Can't copy to nullptr stream.");
        }
        if (buffer_size <= 0)
        {
            throw std::out_of_range("Can't copy to a stream with buffer size lower than or equal to 0.");
        }

        return task::run([=] { return copy_to(s, buffer_size); });
    }

    std::future<void> stream::flush_async()
    {
        return task::run(std::bind(&stream::flush, this));
    }

    std::future<std::streamsize> stream::read_async(gsl::span<uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Read buffer is a nullptr.");
        }

        return task::run([=] { return read(buffer); });
    }

    int32_t stream::read_byte()
    {
        uint8_t b;
        auto r = read(gsl::span<uint8_t>(&b, 1));
        return r == 0 ? -1 : b;
    }

    std::future<void> stream::write_async(gsl::span<const uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Write buffer is a nullptr.");
        }

        return task::run([=] { return write(buffer); });
    }

    void stream::write_byte(uint8_t value)
    {
        write(gsl::span<uint8_t>(&value, 1));
    }
}
