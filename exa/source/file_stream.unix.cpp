#ifndef _WIN32
#include <exa/file_stream.hpp>
#include <exa/enum_flag.hpp>

namespace exa
{
    struct file_stream::file_stream_context
    {
        int file;
        file_mode mode;
        file_access access;
        file_share share;
        std::string name;
    };

    namespace
    {
        void validate_descriptor(int descriptor)
        {
            if (descriptor == -1)
            {
                throw std::runtime_error("Invalid file descriptor.");
            }
        }

        int file_enum_to_flags(file_mode mode, file_access access, file_share share, file_options options)
        {
            int flags = 0;

            switch (mode)
            {
                case file_mode::open:
                    break;
                case file_mode::open_or_create:
                    flags |= O_CREAT;
                    break;
                case file_mode::truncate:
                    flags |= O_TRUNC;
                    break;
                case file_mode::append:
                    flags |= O_CREAT | O_APPEND;
                    break;
                case file_mode::create:
                    flags |= O_CREAT | O_TRUNC;
                    break;
                case file_mode::create_new:
                    flags |= O_CREAT | O_EXCL;
                    break;
                default:
                    break;
            }

            switch (access)
            {
                case file_access::read:
                    flags |= O_RDONLY;
                    break;
                case file_access::write:
                    flags |= O_RDWR;
                    break;
                case file_access::read_write:
                    flags |= O_WRONLY;
                    break;
                default:
                    break;
            }

            if (has_flag(options, file_options::write_through))
            {
                flags |= O_SYNC;
            }

            return flags;
        }
    }

    file_stream::file_stream(const std::string& path, file_mode mode, file_access access, file_share share,
                             file_options options)
    {
        if (path.empty())
        {
            throw std::invalid_argument("Given path is empty.");
        }

        if (!has_flag(access, file_access::write))
        {
            switch (mode)
            {
                case file_mode::append:
                case file_mode::create:
                case file_mode::create_new:
                case file_mode::truncate:
                    throw std::invalid_argument("Given file mode requires write access.");
                default:
                    break;
            }
        }
        if (has_flag(access, file_access::read) && mode == file_mode::append)
        {
            throw std::invalid_argument("Append mode doesn't allow read access.");
        }

        context_ = std::make_unique<file_stream_context>();
        context_->access = access;
        context_->share = share;
        context_->mode = mode;
        context_->name = path;

        auto flags = file_enum_to_flags(mode, access, share, options);
        auto permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

        context_->file = open(path.c_str(), flags, permission);

        if (context_->file == -1)
        {
            throw std::system_error(errno, std::system_category(), "open");
        }

        int lock = share == file_share::none ? LOCK_EX : LOCK_SH;

        if (flock(context_->file, lock | LOCK_NB) == -1 && errno == EWOULDBLOCK)
        {
            close();
            throw std::system_error(errno, std::system_category(), "flock");
        }

        auto fadv = has_flag(options, file_options::random_access)
                        ? POSIX_FADV_RANDOM
                        : (has_flag(options, file_options::sequential_scan) ? POSIX_FADV_SEQUENTIAL : 0);
        auto result = posix_fadvise(context_->file, 0, 0, fadv);

        if (result != 0)
        {
            close();
            throw std::system_error(result, std::system_category(), "posix_fadvise");
        }
    }

    file_stream::~file_stream()
    {
        close();
    }

    bool file_stream::can_read() const
    {
        validate_descriptor(context_->file);
        return has_flag(context_->access, file_access::read);
    }

    bool file_stream::can_seek() const
    {
        validate_descriptor(context_->file);
        return context_->mode != file_mode::append;
    }

    bool file_stream::can_write() const
    {
        validate_descriptor(context_->file);
        return has_flag(context_->access, file_access::write);
    }

    std::streamsize file_stream::size() const
    {
        validate_descriptor(context_->file);
        struct stat s = {0};

        if (fstat(context_->file, &s) == -1)
        {
            throw std::system_error(errno, std::system_category(), "fstat");
        }

        return static_cast<std::streamsize>(s.st_size);
    }

    void file_stream::size(std::streamsize value)
    {
        if (value < 0)
        {
            throw std::out_of_range("Length of buffered stream can't be negative.");
        }

        validate_descriptor(context_->file);

        auto pos = position();
        position(value);

        if (ftruncate(context_->file, static_cast<off_t>(value)) == -1)
        {
            throw std::system_error(errno, std::system_category(), "ftruncate");
        }

        if (pos < value)
        {
            position(pos);
        }
        else
        {
            seek(0, seek_origin::end);
        }
    }

    std::streamoff file_stream::position() const
    {
        if (value < 0)
        {
            throw std::out_of_range("Can't set a negative stream position.");
        }

        validate_descriptor(context_->file);
        auto pos = lseek(context_->file, 0, SEEK_CUR);
        return static_cast<std::streamoff>(pos);
    }

    void file_stream::position(std::streamoff value)
    {
        validate_descriptor(context_->file);

        if (lseek(context_->file, static_cast<off_t>(value), SEEK_SET) == -1)
        {
            throw std::system_error(errno, std::system_category(), "lseek");
        }
    }

    void file_stream::close()
    {
        if (context_->file != -1)
        {
            ::close(context_->file);
            context_->file = -1;
        }
    }

    void file_stream::flush()
    {
        validate_descriptor(context_->file);

        if (fsync(context_->file) == -1)
        {
            throw std::system_error(errno, std::system_category(), "fsync");
        }
    }

    std::streamsize file_stream::read(gsl::span<uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Read buffer is a nullptr.");
        }

        validate_descriptor(context_->file);

        auto n = ::read(context_->file, buffer.data(), static_cast<size_t>(buffer.size()));

        if (n == -1)
        {
            throw std::system_error(errno, std::system_category(), "read");
        }
        else
        {
            return static_cast<std::streamsize>(n);
        }
    }

    std::streamoff file_stream::seek(std::streamoff offset, seek_origin origin)
    {
        validate_descriptor(context_->file);

        int whence = 0;

        switch (origin)
        {
            case seek_origin::begin:
                whence = SEEK_SET;
                break;
            case seek_origin::current:
                whence = SEEK_CUR;
                break;
            case seek_origin::end:
                whence = SEEK_END;
                break;
            default:
                break;
        }

        auto pos = lseek(context_->file, static_cast<off_t>(offset), whence);

        if (pos == -1)
        {
            throw std::system_error(errno, std::system_category(), "lseek");
        }
        else
        {
            return static_cast<std::streamoff>(pos);
        }
    }

    void file_stream::write(gsl::span<const uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Write buffer is a nullptr.");
        }

        validate_descriptor(context_->file);

        auto n = ::write(context_->file, buffer.data(), static_cast<size_t>(buffer.size()));

        if (n == -1)
        {
            throw std::system_error(errno, std::system_category(), "write");
        }
        if (n != buffer.size())
        {
            throw std::runtime_error("Not all bytes were written to the file stream.");
        }
    }

    const std::string& file_stream::name() const
    {
        validate_descriptor(context_->file);
        return context_->name;
    }
}

#endif
