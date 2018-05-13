#pragma once

#include <exa/dependencies.hpp>

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <future>
#include <ios>
#include <memory>

namespace exa
{
    enum class seek_origin
    {
        begin,
        current,
        end
    };

    class stream
    {
    public:
        static constexpr std::streamsize default_buffer_size = 81920;

        virtual ~stream() = default;

        virtual bool can_read() const;

        virtual bool can_seek() const;

        virtual bool can_timeout() const;

        virtual bool can_write() const;

        virtual std::streamsize size() const;

        virtual std::streamoff position() const;

        virtual void position(std::streamoff value);

        virtual std::chrono::milliseconds read_timeout() const;

        virtual void read_timeout(const std::chrono::milliseconds& value);

        virtual std::chrono::milliseconds write_timeout() const;

        virtual void write_timeout(const std::chrono::milliseconds& value);

        virtual void close();

        virtual void copy_to(std::shared_ptr<stream> s);

        virtual void copy_to(std::shared_ptr<stream> s, std::streamsize buffer_size);

        virtual std::future<void> copy_to_async(std::shared_ptr<stream> s);

        virtual std::future<void> copy_to_async(std::shared_ptr<stream> s, std::streamsize buffer_size);

        virtual void flush() = 0;

        virtual std::future<void> flush_async();

        virtual std::streamsize read(gsl::span<uint8_t> buffer) = 0;

        virtual std::future<std::streamsize> read_async(gsl::span<uint8_t> buffer);

        virtual int32_t read_byte();

        virtual std::streamoff seek(std::streamoff offset, seek_origin origin) = 0;

        virtual void set_length(std::streamoff value) = 0;

        virtual void write(gsl::span<const uint8_t> buffer) = 0;

        virtual std::future<void> write_async(gsl::span<const uint8_t> buffer);

        virtual void write_byte(uint8_t value);
    };
}
