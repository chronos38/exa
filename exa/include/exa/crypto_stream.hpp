#pragma once

#include <exa/stream.hpp>
#include <exa/crypto_transform.hpp>

namespace exa
{
    enum class crypto_stream_mode
    {
        read,
        write
    };

    class crypto_stream : public stream
    {
    public:
        crypto_stream(const std::shared_ptr<stream>& stream, const std::shared_ptr<crypto_transform>& transform,
                      crypto_stream_mode mode);
        virtual ~crypto_stream();

        // Inherited via stream
        virtual bool can_read() const override;
        virtual bool can_seek() const override;
        virtual bool can_write() const override;
        virtual std::streamsize size() const override;
        virtual void size(std::streamsize value) override;
        virtual std::streamoff position() const override;
        virtual void position(std::streamoff value) override;
        virtual void flush() override;
        virtual std::streamsize read(gsl::span<uint8_t> buffer) override;
        virtual std::streamoff seek(std::streamoff offset, seek_origin origin) override;
        virtual void write(gsl::span<const uint8_t> buffer) override;

    private:
        class stream_transform_filter;

        crypto_stream_mode mode_;
        std::shared_ptr<stream> stream_;
        std::unique_ptr<stream_transform_filter> transform_;
    };
}
