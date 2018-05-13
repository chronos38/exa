#include <exa/buffered_stream.hpp>

namespace exa
{
    buffered_stream::buffered_stream(const std::shared_ptr<stream>& s, std::streamsize buffer_size) : stream_(s)
    {
        if (buffer_size <= 0)
        {
            throw std::out_of_range("Buffer size for buffered stream needs to greater than 0.");
        }

        buffer_.reserve(static_cast<size_t>(buffer_size));
    }

    bool buffered_stream::can_read() const
    {
        return stream_->can_read();
    }

    bool buffered_stream::can_seek() const
    {
        return true;
    }

    bool buffered_stream::can_write() const
    {
        return stream_->can_write();
    }

    std::streamoff buffered_stream::size() const
    {
        return static_cast<std::streamoff>(buffer_.size());
    }

    std::streamoff buffered_stream::position() const
    {
        return position_;
    }

    void buffered_stream::position(std::streamoff value)
    {
        position_ = value;
    }

    void buffered_stream::close()
    {
        stream_->close();
    }

    void buffered_stream::flush()
    {
        stream_->flush();
    }

    std::streamsize buffered_stream::read(gsl::span<uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Read buffer is a nullptr.");
        }
        if (static_cast<size_t>(buffer.size()) >= buffer_.capacity())
        {
            return stream_->read(buffer);
        }
        else
        {
            return read_buffered(buffer);
        }
    }

    std::streamoff buffered_stream::seek(std::streamoff offset, seek_origin origin)
    {
        switch (origin)
        {
            case seek_origin::begin:
                position_ = offset;
                break;
            case seek_origin::current:
                position_ = position_ + offset;
                break;
            case seek_origin::end:
                position_ = buffer_.size() + offset;
                break;
            default:
                break;
        }

        return position_;
    }

    void buffered_stream::set_length(std::streamoff value)
    {
        if (value < 0)
        {
            throw std::out_of_range("Length of buffered stream can't be negative.");
        }

        buffer_.resize(static_cast<size_t>(value));
    }

    void buffered_stream::write(gsl::span<const uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Write buffer is a nullptr.");
        }
        if (static_cast<size_t>(buffer.size()) >= buffer_.capacity())
        {
            stream_->write(buffer);
        }
        else
        {
            write_buffered(buffer);
        }
    }

    std::streamsize buffered_stream::read_buffered(gsl::span<uint8_t> buffer)
    {
        if (context_ != buffer_context::read || position_ >= static_cast<std::streamoff>(buffer_.size()))
        {
            context_ = buffer_context::read;
            buffer_.resize(buffer_.capacity());
            auto n = stream_->read(buffer_);
            buffer_.resize(static_cast<size_t>(n));
            position_ = 0;
        }

        auto n = std::min<std::streamsize>(buffer_.size() - position_, buffer.size());
        std::copy_n(std::begin(buffer_), n, std::begin(buffer));
        position_ += static_cast<std::streamoff>(n);
        return n;
    }

    void buffered_stream::write_buffered(gsl::span<const uint8_t> buffer)
    {
        if (context_ != buffer_context::write)
        {
            context_ = buffer_context::write;
            buffer_.resize(0);
            position_ = 0;
        }

        if (position_ + buffer.size() > static_cast<std::streamoff>(buffer_.capacity()))
        {
            stream_->write(gsl::span<const uint8_t>(buffer_));
            buffer_.resize(0);
            position_ = 0;
        }

        buffer_.resize(static_cast<size_t>(position_ + buffer.size()));
        auto it = std::next(std::begin(buffer_), static_cast<std::ptrdiff_t>(position_));
        std::copy(std::begin(buffer), std::end(buffer), it);
        position_ += buffer.size();
    }
}
