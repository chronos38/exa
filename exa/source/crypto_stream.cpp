#include <exa/crypto_stream.hpp>
#include <exa/memory_stream.hpp>

#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/simple.h>

#include <algorithm>

namespace exa
{
    namespace
    {
        class stream_sink : public CryptoPP::Bufferless<CryptoPP::Sink>
        {
        public:
            virtual ~stream_sink() = default;

            stream_sink(const std::shared_ptr<exa::stream>& s, crypto_stream_mode m) : stream_(s), mode_(m)
            {
                if (s == nullptr)
                {
                    throw std::invalid_argument("Stream sink needs non null stream.");
                }
            }

            size_t Put2(const byte* i, size_t size, int message_end, bool blocking)
            {
                CRYPTOPP_UNUSED(message_end);
                CRYPTOPP_UNUSED(blocking);

                if (size > 0)
                {
                    stream_->write(gsl::make_span(i, size));
                }

                return 0;
            }

            const std::shared_ptr<stream>& stream() const
            {
                return stream_;
            }

        private:
            std::shared_ptr<exa::stream> stream_;
            crypto_stream_mode mode_;
        };
    }

    class crypto_stream::stream_transform_filter : public CryptoPP::StreamTransformationFilter
    {
    public:
        stream_transform_filter(CryptoPP::StreamTransformation& c, stream_sink* attachment = nullptr,
                                BlockPaddingScheme padding = DEFAULT_PADDING)
            : StreamTransformationFilter(c, attachment, padding), sink_(attachment)
        {
        }

        stream_sink* sink()
        {
            return sink_;
        }

        const stream_sink* sink() const
        {
            return sink_;
        }

    private:
        stream_sink* sink_ = nullptr;
    };

    crypto_stream::crypto_stream(const std::shared_ptr<stream>& stream, const std::shared_ptr<crypto_transform>& transform,
                                 crypto_stream_mode mode)
        : mode_(mode), stream_(stream)
    {
        if (stream == nullptr)
        {
            throw std::invalid_argument("Crypto stream needs non null stream.");
        }
        if (transform == nullptr)
        {
            throw std::invalid_argument("Crypto stream needs non null transform.");
        }

        auto&& algorithm = transform->stream_transformation();

        switch (mode)
        {
            case crypto_stream_mode::read:
            {
                if (!stream->can_read())
                {
                    throw std::invalid_argument("Crypto stream need readable stream for read mode.");
                }

                auto buffer = std::make_shared<memory_stream>();
                transform_ = std::make_unique<stream_transform_filter>(algorithm, new stream_sink(buffer, mode));
                break;
            }
            case crypto_stream_mode::write:
                if (!stream->can_write())
                {
                    throw std::invalid_argument("Crypto stream need writable stream for write mode.");
                }

                transform_ = std::make_unique<stream_transform_filter>(algorithm, new stream_sink(stream, mode));
                break;
            default:
                break;
        }
    }

    crypto_stream::~crypto_stream()
    {
    }

    bool crypto_stream::can_read() const
    {
        return mode_ == crypto_stream_mode::read;
    }

    bool crypto_stream::can_seek() const
    {
        return false;
    }

    bool crypto_stream::can_write() const
    {
        return mode_ == crypto_stream_mode::write;
    }

    std::streamsize crypto_stream::size() const
    {
        throw std::runtime_error("Size not supported by crypto stream.");
    }

    void crypto_stream::size(std::streamsize value)
    {
        throw std::runtime_error("Size not supported by crypto stream.");
    }

    std::streamoff crypto_stream::position() const
    {
        throw std::runtime_error("Position not supported by crypto stream.");
    }

    void crypto_stream::position(std::streamoff value)
    {
        throw std::runtime_error("Position not supported by crypto stream.");
    }

    void crypto_stream::flush()
    {
    }

    std::streamsize crypto_stream::read(gsl::span<uint8_t> buffer)
    {
        if (mode_ != crypto_stream_mode::read)
        {
            throw std::runtime_error("Crypto stream doesn't support reading.");
        }

        std::vector<uint8_t> v(buffer.size());
        auto n = stream_->read(v);
        v.resize(static_cast<size_t>(n));

        auto&& s = std::dynamic_pointer_cast<memory_stream>(transform_->sink()->stream());
        s->size(buffer.size());
        s->position(0);
        transform_->Put(v.data(), v.size());

        auto&& b = s->buffer();
        auto last = next(begin(b), static_cast<ptrdiff_t>(n));
        std::copy(begin(b), last, buffer.begin());

        return n;
    }

    std::streamoff crypto_stream::seek(std::streamoff offset, seek_origin origin)
    {
        throw std::runtime_error("Seek not supported by crypto stream.");
    }

    void crypto_stream::write(gsl::span<const uint8_t> buffer)
    {
        if (mode_ != crypto_stream_mode::write)
        {
            throw std::runtime_error("Crypto stream doesn't support reading.");
        }

        auto n = transform_->Put(buffer.data(), static_cast<size_t>(buffer.size()));

        if (n != 0)
        {
            throw std::runtime_error("Transformation of some bytes failed.");
        }
    }
}
