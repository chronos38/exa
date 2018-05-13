#pragma once

#include <exa/stream.hpp>

#include <string>
#include <memory>
#include <cstdint>

namespace exa
{
    enum class file_mode
    {
        append = 0,
        create = 2,
        create_new = 1,
        open = 3,
        open_or_create = 4,
        truncate = 5
    };

    enum class file_access
    {
        read = 1,
        write = 2,
        read_write = read | write
    };

    enum class file_share
    {
        none = 0,
        read = 1,
        write = 2,
        read_write = read | write,
        remove = 4
    };

    enum class file_options : uint32_t
    {
        none = 0,
        asynchronous = 0x40000000,
        delete_on_close = 0x04000000,
        encrypted = 0x00004000,
        random_access = 0x10000000,
        sequential_scan = 0x08000000,
        write_through = 0x80000000
    };

    class file_stream : public stream
    {
    public:
        file_stream() = default;
        file_stream(const file_stream&) = delete;
        file_stream(const std::string& path, file_mode mode, file_access access = file_access::read_write,
                    file_share share = file_share::read, file_options options = file_options::none);
        virtual ~file_stream();

        // Inherited via stream
        virtual bool can_read() const override;

        virtual bool can_seek() const override;

        virtual bool can_write() const override;

        virtual std::streamsize size() const override;

        virtual std::streamoff position() const override;

        virtual void position(std::streamoff value) override;

        virtual void close() override;

        virtual void flush() override;

        virtual std::streamsize read(gsl::span<uint8_t> buffer) override;

        virtual std::streamoff seek(std::streamoff offset, seek_origin origin) override;

        virtual void set_length(std::streamoff value) override;

        virtual void write(gsl::span<const uint8_t> buffer) override;

        // Specific to file_stream
        std::string name() const;

    private:
        struct file_stream_context;
        std::unique_ptr<file_stream_context> context_;
    };
}
