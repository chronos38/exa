#pragma once

#include <exa/stream.hpp>

#include <vector>

namespace exa
{
    class memory_stream : public stream
    {
    public:
        memory_stream() = default;

        explicit memory_stream(std::streamsize count);

        explicit memory_stream(gsl::span<uint8_t> b);

        explicit memory_stream(const std::vector<uint8_t>& b);

        virtual ~memory_stream() = default;

        // Inherited via stream
        virtual bool can_read() const override;

        virtual bool can_seek() const override;

        virtual bool can_write() const override;

        virtual std::streamsize size() const override;

        virtual void size(std::streamsize value) override;

        virtual std::streamoff position() const override;

        virtual void position(std::streamoff value) override;

        virtual void flush() override;

        virtual std::streamsize read(gsl::span<uint8_t> b) override;

        virtual std::streamoff seek(std::streamoff offset, seek_origin origin) override;

        virtual void write(gsl::span<const uint8_t> b) override;

        // Specific to memory_stream
        const std::vector<uint8_t>& buffer() const;

        virtual std::vector<uint8_t> to_array();

        size_t capacity() const;

        void capacity(size_t n);

    private:
        std::vector<uint8_t> buffer_;
        std::streamoff position_ = 0;
        bool resizable_ = true;
    };
}
