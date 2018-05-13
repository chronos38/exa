#pragma once

#include <exa/stream.hpp>
#include <exa/dependencies.hpp>
#include <exa/file_stream.hpp>
#include <exa/socket.hpp>

namespace exa
{
    class network_stream : public stream
    {
    public:
        network_stream() = delete;
        network_stream(const network_stream&) = delete;
        network_stream(const std::shared_ptr<socket>& socket, bool owns = false);
        network_stream(const std::shared_ptr<socket>& socket, file_access access, bool owns = false);
        virtual ~network_stream();

        // Inherited via stream
        virtual bool can_read() const override;

        virtual bool can_seek() const override;

        virtual bool can_timeout() const override;

        virtual bool can_write() const override;

        virtual std::streamsize size() const override;

        virtual std::streamoff position() const override;

        virtual void position(std::streamoff value) override;

        virtual std::chrono::milliseconds read_timeout() const override;

        virtual void read_timeout(const std::chrono::milliseconds& value) override;

        virtual std::chrono::milliseconds write_timeout() const override;

        virtual void write_timeout(const std::chrono::milliseconds& value) override;

        virtual bool readable() const;

        virtual void readable(bool value);

        virtual bool writable() const;

        virtual void writable(bool value);

        virtual void close() override;

        virtual std::future<void> copy_to_async(std::shared_ptr<stream> s, std::streamsize buffer_size = 81920) override;

        virtual void flush() override;

        virtual std::future<void> flush_async() override;

        virtual std::streamsize read(gsl::span<uint8_t> buffer) override;

        virtual std::future<std::streamsize> read_async(gsl::span<uint8_t> buffer) override;

        virtual std::streamoff seek(std::streamoff offset, seek_origin origin) override;

        virtual void set_length(std::streamoff value) override;

        virtual void write(gsl::span<const uint8_t> buffer) override;

        virtual std::future<void> write_async(gsl::span<const uint8_t> buffer) override;

        // Specific to network_stream
        bool data_available() const;

        const std::shared_ptr<socket>& socket() const;

    private:
        std::shared_ptr<exa::socket> socket_;
        bool owns_;
        bool readable_ = false;
        bool writable_ = false;
    };
}
