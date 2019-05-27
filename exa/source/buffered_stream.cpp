#include <exa/buffered_stream.hpp>

#include <algorithm>

namespace exa
{
    buffered_stream::buffered_stream(stream* s, std::streamsize buffer_size) : stream_(s), buffer_size_(buffer_size)
    {
        if (s == nullptr)
        {
            throw std::invalid_argument("Given stream for buffered stream is nullptr.");
        }
        if (buffer_size <= 0)
        {
            throw std::out_of_range("Buffer size for buffered stream needs to greater than 0.");
        }
        if (!s->can_read() && !s->can_write())
        {
            throw std::invalid_argument("Stream needs to be both read and writable.");
        }

        buffer_.resize(static_cast<size_t>(buffer_size_));
    }

    bool buffered_stream::can_read() const
    {
        return stream_->can_read();
    }

    bool buffered_stream::can_seek() const
    {
        return stream_->can_seek();
    }

    bool buffered_stream::can_write() const
    {
        return stream_->can_write();
    }

    std::streamoff buffered_stream::size() const
    {
        return stream_->size();
    }

    void buffered_stream::size(std::streamsize value)
    {
        if (value < 0)
        {
            throw std::out_of_range("Length of buffered stream can't be negative.");
        }

        flush();
        stream_->size(value);
    }

    std::streamoff buffered_stream::position() const
    {
        return stream_->position() + (read_pos_ - read_size_ + write_pos_);
    }

    void buffered_stream::position(std::streamoff value)
    {
        if (value < 0)
        {
            throw std::out_of_range("Can't set a negative stream position.");
        }
        if (write_pos_ > 0)
        {
            flush_write();
        }

        read_pos_ = 0;
        read_size_ = 0;
        stream_->seek(value, seek_origin::begin);
    }

    void buffered_stream::flush()
    {
        if (write_pos_ > 0)
        {
            flush_write();
        }
        else if (read_pos_ < read_size_)
        {
            if (stream_->can_seek())
            {
                flush_read();
            }
            if (stream_->can_write())
            {
                stream_->flush();
            }
        }
        else
        {
            if (stream_->can_write())
            {
                stream_->flush();
            }

            write_pos_ = 0;
            read_pos_ = 0;
        }
    }

    std::streamsize buffered_stream::read(gsl::span<uint8_t> b)
    {
        if (b.data() == nullptr)
        {
            throw std::invalid_argument("Read buffer is a nullptr.");
        }

        auto n = read_buffered(b);

        if (n == static_cast<std::streamsize>(b.size()))
        {
            return n;
        }

        auto already_read = n;

        if (n > 0)
        {
            b = gsl::span<uint8_t>(b.data() + n, b.size() - static_cast<ptrdiff_t>(n));
        }

        read_pos_ = 0;
        read_size_ = 0;

        if (write_pos_ > 0)
        {
            flush_write();
        }
        if (b.size() >= buffer_size_)
        {
            return stream_->read(b) + already_read;
        }

        read_size_ = stream_->read(buffer_);
        n = read_buffered(b);
        return already_read + n;
    }

    std::streamoff buffered_stream::seek(std::streamoff offset, seek_origin origin)
    {
        if (write_pos_ > 0)
        {
            flush_write();
            return stream_->seek(offset, origin);
        }
        if (read_size_ - read_pos_ > 0 && origin == seek_origin::current)
        {
            offset -= (read_size_ - read_pos_);
        }

        auto old_pos = position();
        auto new_pos = stream_->seek(offset, origin);
        read_pos_ = new_pos - (old_pos - read_pos_);

        if (read_pos_ >= 0 && read_pos_ < read_size_)
        {
            stream_->seek(read_size_ - read_pos_, seek_origin::current);
        }
        else
        {
            read_pos_ = 0;
            read_size_ = 0;
        }

        return new_pos;
    }

    void buffered_stream::write(gsl::span<const uint8_t> b)
    {
        if (b.data() == nullptr)
        {
            throw std::invalid_argument("Read buffer is a nullptr.");
        }

        if (write_pos_ == 0)
        {
            clear_read_buffer();
        }

        auto total_bytes = write_pos_ + b.size();

        if (total_bytes < buffer_size_)
        {
            write_buffered(b);

            if (write_pos_ >= buffer_size_)
            {
                stream_->write(buffer_);
                write_pos_ = 0;
                write_buffered(b);
            }
        }
        else
        {
            if (write_pos_ > 0 && write_pos_ < buffer_size_)
            {
                stream_->write(gsl::span<uint8_t>(buffer_.data(), static_cast<ptrdiff_t>(write_pos_)));
                write_pos_ = 0;
            }

            stream_->write(b);
        }
    }

    stream* buffered_stream::underlying_stream() const
    {
        return stream_;
    }

    std::streamsize buffered_stream::buffer_size() const
    {
        return buffer_size_;
    }

    void buffered_stream::flush_write()
    {
        stream_->write(gsl::span<uint8_t>(buffer_.data(), static_cast<ptrdiff_t>(write_pos_)));
        stream_->flush();
        write_pos_ = 0;
    }

    void buffered_stream::flush_read()
    {
        if (read_pos_ - read_size_ != 0)
        {
            stream_->seek(read_pos_ - read_size_, seek_origin::current);
        }

        read_size_ = 0;
        read_pos_ = 0;
    }

    std::streamsize buffered_stream::read_buffered(gsl::span<uint8_t> b)
    {
        auto n = read_size_ - read_pos_;

        if (n <= 0)
        {
            return 0;
        }

        if (n > b.size())
        {
            n = b.size();
        }

        std::copy(std::next(std::begin(buffer_), static_cast<ptrdiff_t>(read_pos_)),
                  std::next(std::begin(buffer_), static_cast<ptrdiff_t>(read_pos_ + n)), std::begin(b));
        read_pos_ += n;
        return n;
    }

    void buffered_stream::write_buffered(gsl::span<const uint8_t>& b)
    {
        auto n = std::min<std::streamsize>(buffer_size_ - write_pos_, b.size());

        if (n > 0)
        {
            std::copy(std::begin(b), std::next(std::begin(b), static_cast<ptrdiff_t>(n)),
                      std::next(std::begin(buffer_), static_cast<ptrdiff_t>(write_pos_)));
            write_pos_ += n;
            b = gsl::span<const uint8_t>(b.data() + n, b.size() - static_cast<ptrdiff_t>(n));
        }
    }

    void buffered_stream::clear_read_buffer()
    {
        if (read_pos_ == read_size_)
        {
            read_pos_ = 0;
            read_size_ = 0;
        }
        else
        {
            flush_read();
        }
    }
}
