#pragma once

#include <exa/stream.hpp>

#include <vector>

namespace exa
{
    class buffered_stream final : public stream
    {
    public:
        buffered_stream() = default;
        explicit buffered_stream(const std::shared_ptr<stream>& s, std::streamsize buffer_size = 4096);
        virtual ~buffered_stream() = default;

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

    private:
        std::streamsize read_buffered(gsl::span<uint8_t> buffer);

        void write_buffered(gsl::span<const uint8_t> buffer);

        enum class buffer_context
        {
            none,
            read,
            write
        };

        std::shared_ptr<stream> stream_;
        std::vector<uint8_t> buffer_;
        std::streamoff position_ = 0;
        buffer_context context_ = buffer_context::none;
    };
}
