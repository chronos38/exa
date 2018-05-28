#ifdef _WIN32
#include <exa/file_stream.hpp>
#include <exa/enum_flag.hpp>
#include <exa/concepts.hpp>

namespace exa
{
    struct file_stream::file_stream_context
    {
        HANDLE file;
        file_mode mode;
        file_access access;
        file_share share;
        std::string name;
    };

    namespace
    {
        void validate_handle(HANDLE handle)
        {
            if (handle == INVALID_HANDLE_VALUE)
            {
                throw std::runtime_error("Invalid file handle.");
            }
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

        if (mode == file_mode::append)
        {
            mode = file_mode::open_or_create;
        }

        DWORD share_mode = (has_flag(share, file_share::read) ? FILE_SHARE_READ : 0) |
                           (has_flag(share, file_share::write) ? FILE_SHARE_WRITE : 0) |
                           (has_flag(share, file_share::remove) ? FILE_SHARE_DELETE : 0);
        DWORD desired_access = context_->mode == file_mode::append
                                   ? FILE_APPEND_DATA
                                   : (has_flag(access, file_access::read) ? GENERIC_READ : 0) |
                                         (has_flag(access, file_access::write) ? GENERIC_WRITE : 0);
        DWORD creation_disposition = static_cast<DWORD>(static_cast<std::underlying_type_t<file_mode>>(mode));
        DWORD flags = static_cast<DWORD>(static_cast<std::underlying_type_t<file_options>>(options));
        flags |= SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS;

        auto size = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
        auto wstr = std::make_unique<wchar_t[]>(size);
        auto rc = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), static_cast<int>(path.size()), wstr.get(), size);

        if (rc == 0)
        {
            throw std::system_error(GetLastError(), std::system_category(), "MultiByteToWideChar");
        }

        context_->file = CreateFile(wstr.get(), desired_access, share_mode, nullptr, creation_disposition, flags, nullptr);

        if (context_->file == INVALID_HANDLE_VALUE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "CreateFile");
        }
    }

    file_stream::~file_stream()
    {
        close();
    }

    bool file_stream::can_read() const
    {
        validate_handle(context_->file);
        return has_flag(context_->access, file_access::read);
    }

    bool file_stream::can_seek() const
    {
        validate_handle(context_->file);
        return context_->mode != file_mode::append;
    }

    bool file_stream::can_write() const
    {
        validate_handle(context_->file);
        return has_flag(context_->access, file_access::write);
    }

    std::streamsize file_stream::size() const
    {
        validate_handle(context_->file);

        LARGE_INTEGER value;

        if (GetFileSizeEx(context_->file, &value) == FALSE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "GetFileSizeEx");
        }

        return static_cast<std::streamsize>(value.QuadPart);
    }

    void file_stream::size(std::streamsize value)
    {
        if (value < 0)
        {
            throw std::out_of_range("Length of buffered stream can't be negative.");
        }

        validate_handle(context_->file);

        auto pos = position();
        position(value);

        if (SetEndOfFile(context_->file) == FALSE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "SetEndOfFile");
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
        validate_handle(context_->file);

        DWORD method = FILE_CURRENT;
        LARGE_INTEGER distance;
        LARGE_INTEGER location;
        distance.QuadPart = 0;

        if (SetFilePointerEx(context_->file, distance, &location, method) == FALSE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "SetFilePointerEx");
        }
        else
        {
            return static_cast<std::streamoff>(location.QuadPart);
        }
    }

    void file_stream::position(std::streamoff value)
    {
        if (value < 0)
        {
            throw std::out_of_range("Can't set a negative stream position.");
        }

        validate_handle(context_->file);

        DWORD method = FILE_BEGIN;
        LARGE_INTEGER distance;
        distance.QuadPart = static_cast<LONGLONG>(value);

        if (SetFilePointerEx(context_->file, distance, nullptr, method) == FALSE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "SetFilePointerEx");
        }
    }

    void file_stream::close()
    {
        if (context_->file != INVALID_HANDLE_VALUE)
        {
            CloseHandle(context_->file);
            context_->file = INVALID_HANDLE_VALUE;
        }
    }

    void file_stream::flush()
    {
        validate_handle(context_->file);
        FlushFileBuffers(context_->file);
    }

    std::streamsize file_stream::read(gsl::span<uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Read buffer is a nullptr.");
        }

        validate_handle(context_->file);
        DWORD n = 0;

        if (ReadFile(context_->file, buffer.data(), static_cast<DWORD>(buffer.size()), &n, nullptr) == FALSE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "ReadFile");
        }
        else
        {
            return static_cast<std::streamsize>(n);
        }
    }

    std::streamoff file_stream::seek(std::streamoff offset, seek_origin origin)
    {
        validate_handle(context_->file);
        BOOL result = FALSE;
        LARGE_INTEGER distance;
        LARGE_INTEGER location;
        DWORD method;
        distance.QuadPart = static_cast<LONGLONG>(offset);

        switch (origin)
        {
            case seek_origin::begin:
                method = FILE_BEGIN;
                result = SetFilePointerEx(context_->file, distance, &location, method);
                break;
            case seek_origin::current:
                method = FILE_CURRENT;
                result = SetFilePointerEx(context_->file, distance, &location, method);
                break;
            case seek_origin::end:
                method = FILE_END;
                result = SetFilePointerEx(context_->file, distance, &location, method);
                break;
            default:
                throw std::invalid_argument("Seek origin has invalid value.");
        }

        if (result == FALSE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "SetFilePointerEx");
        }

        return static_cast<std::streamoff>(location.QuadPart);
    }

    void file_stream::write(gsl::span<const uint8_t> buffer)
    {
        if (buffer.data() == nullptr)
        {
            throw std::invalid_argument("Write buffer is a nullptr.");
        }

        validate_handle(context_->file);
        DWORD n = 0;

        if (WriteFile(context_->file, buffer.data(), static_cast<DWORD>(buffer.size()), &n, nullptr) == FALSE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "WriteFile");
        }
        if (n != static_cast<DWORD>(buffer.size()))
        {
            throw std::runtime_error("Not all bytes were written to the file stream.");
        }
    }

    const std::string& file_stream::name() const
    {
        validate_handle(context_->file);
        return context_->name;
    }
}

#endif
