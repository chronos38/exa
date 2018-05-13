#include <exa/memory_stream.hpp>

#include <algorithm>

namespace exa
{
    memory_stream::memory_stream(std::streamsize count)
    {
        buffer_.reserve(static_cast<size_t>(count));
    }

    memory_stream::memory_stream(gsl::span<uint8_t> b) : buffer_(b.begin(), b.end())
    {
        resizable_ = false;
    }

    memory_stream::memory_stream(const std::vector<uint8_t>& b) : buffer_(b)
    {
        resizable_ = false;
    }

    bool memory_stream::can_read() const
    {
        return true;
    }

    bool memory_stream::can_seek() const
    {
        return true;
    }

    bool memory_stream::can_write() const
    {
        return true;
    }

    std::streamsize memory_stream::size() const
    {
        return static_cast<std::streamoff>(buffer_.size());
    }

    std::streamoff memory_stream::position() const
    {
        return position_;
    }

    void memory_stream::position(std::streamoff value)
    {
        position_ = value;
    }

    void memory_stream::flush()
    {
    }

    std::streamsize memory_stream::read(gsl::span<uint8_t> b)
    {
        if (b.data() == nullptr)
        {
            throw std::invalid_argument("Read buffer is a nullptr.");
        }

        auto n = std::min<std::streamsize>(buffer_.size() - position_, b.size());
        std::copy_n(std::begin(buffer_), n, std::begin(b));
        position_ += static_cast<std::streamoff>(n);
        return n;
    }

    std::streamoff memory_stream::seek(std::streamoff offset, seek_origin origin)
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

    void memory_stream::set_length(std::streamoff value)
    {
        if (value < 0)
        {
            throw std::out_of_range("Length of memory stream can't be negative.");
        }

        buffer_.resize(static_cast<size_t>(value));
    }

    void memory_stream::write(gsl::span<const uint8_t> b)
    {
        if (b.data() == nullptr)
        {
            throw std::invalid_argument("Write buffer is a nullptr.");
        }

        auto new_position = position_ + b.size();

        if (new_position > static_cast<std::streamoff>(buffer_.size()))
        {
            if (resizable_)
            {
                if (static_cast<size_t>(new_position) > buffer_.capacity())
                {
                    buffer_.reserve(
                        std::max<size_t>(static_cast<size_t>(new_position), static_cast<size_t>(buffer_.size() + 81920)));
                }

                buffer_.resize(static_cast<size_t>(new_position));
            }
            else
            {
                throw std::runtime_error("Memory stream doesn't support resizing.");
            }
        }

        auto it = std::next(std::begin(buffer_), static_cast<std::ptrdiff_t>(position_));
        std::copy(std::begin(b), std::end(b), it);
        position_ += b.size();
    }

    const std::vector<uint8_t>& memory_stream::buffer() const
    {
        return buffer_;
    }

    std::vector<uint8_t> memory_stream::to_array()
    {
        return buffer_;
    }
}
