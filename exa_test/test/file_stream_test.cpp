#include <pch.h>
#include <exa/file_stream.hpp>
#include <exa/concepts.hpp>

using namespace exa;
using namespace testing;
using namespace std::literals::chrono_literals;
using namespace std::literals::string_literals;

namespace
{
    bool fexists(const std::string& filename)
    {
        std::ifstream ifile(filename);
        return static_cast<bool>(ifile);
    }

    void fcreate(const std::string& filename)
    {
        std::fclose(std::fopen(filename.c_str(), "w"));
    }

    void fremove(const std::string& filename)
    {
        std::remove(filename.c_str());
    }

    void fwrite(const std::string& filename, const std::string& data)
    {
        auto f = fopen(filename.c_str(), "a");
        ::fwrite(data.c_str(), sizeof(char), data.size(), f);
        fflush(f);
        fclose(f);
    }
}

class file_stream_test : public Test
{
    virtual void TearDown() override
    {
        fremove("1.txt");
    }
};

TEST_F(file_stream_test, ctor_valid_argument_create_file)
{
    scope(file_stream("1.txt", file_mode::create), [](auto&&) { ASSERT_TRUE(fexists("1.txt")); });
    fremove("1.txt");
    scope(file_stream("1.txt", file_mode::append, file_access::write), [](auto&&) { ASSERT_TRUE(fexists("1.txt")); });
    fremove("1.txt");
    scope(file_stream("1.txt", file_mode::create_new), [](auto&&) { ASSERT_TRUE(fexists("1.txt")); });
    fremove("1.txt");
    scope(file_stream("1.txt", file_mode::open_or_create), [](auto&&) { ASSERT_TRUE(fexists("1.txt")); });
    fremove("1.txt");
}

TEST_F(file_stream_test, ctor_valid_argument_open_existing_file)
{
    fcreate("1.txt");
    ASSERT_NO_THROW(file_stream("1.txt", file_mode::open));
    ASSERT_NO_THROW(file_stream("1.txt", file_mode::truncate));
}

TEST_F(file_stream_test, ctor_invalid_argument_throws)
{
    ASSERT_THROW(file_stream("", file_mode::create), std::invalid_argument);
    ASSERT_THROW(file_stream("", file_mode::append), std::invalid_argument);
    ASSERT_THROW(file_stream("", file_mode::create_new), std::invalid_argument);
    ASSERT_THROW(file_stream("", file_mode::open_or_create), std::invalid_argument);
    ASSERT_THROW(file_stream("", file_mode::open), std::invalid_argument);
    ASSERT_THROW(file_stream("", file_mode::truncate), std::invalid_argument);
    ASSERT_THROW(file_stream("1.txt", file_mode::append, file_access::read), std::invalid_argument);
    ASSERT_THROW(file_stream("1.txt", file_mode::append, file_access::read_write), std::invalid_argument);
    ASSERT_THROW(file_stream("1.txt", file_mode::create, file_access::read), std::invalid_argument);
    ASSERT_THROW(file_stream("1.txt", file_mode::create_new, file_access::read), std::invalid_argument);
    ASSERT_THROW(file_stream("1.txt", file_mode::truncate, file_access::read), std::invalid_argument);
}

TEST_F(file_stream_test, correct_size_and_position_after_writing_data_is_equal)
{
    auto v = "foo\nbar\n42\n"s;
    auto&& s = file_stream("1.txt", file_mode::open_or_create);
    s.write({reinterpret_cast<const uint8_t*>(v.data()), static_cast<ptrdiff_t>(v.size())});
    ASSERT_EQ(v.size(), static_cast<size_t>(s.size()));
    ASSERT_EQ(v.size(), static_cast<size_t>(s.position()));
}

TEST_F(file_stream_test, correct_position_after_seeking)
{
    auto v = "foo\nbar\n42\n"s;
    auto&& s = file_stream("1.txt", file_mode::open_or_create);
    s.write({reinterpret_cast<const uint8_t*>(v.data()), static_cast<ptrdiff_t>(v.size())});
    ASSERT_EQ(v.size(), static_cast<size_t>(s.size()));
    s.seek(4, seek_origin::begin);
    ASSERT_EQ(4, s.position());
}

TEST_F(file_stream_test, correct_size_after_setting_size)
{
    auto&& s = file_stream("1.txt", file_mode::open_or_create);
    s.size(8192);
    ASSERT_EQ(8192, s.size());
}

TEST_F(file_stream_test, correct_position_after_setting_position)
{
    auto&& s = file_stream("1.txt", file_mode::open_or_create);
    s.size(8192);
    ASSERT_EQ(8192, s.size());
    s.position(1024);
    ASSERT_EQ(1024, s.position());
}

TEST_F(file_stream_test, data_equal_after_writing_and_reading)
{
    auto v = "foo\nbar\n42\n"s;
    std::vector<uint8_t> b(v.size());
    auto&& s = file_stream("1.txt", file_mode::open_or_create);
    s.write({reinterpret_cast<const uint8_t*>(v.data()), static_cast<ptrdiff_t>(v.size())});
    s.position(0);
    ASSERT_EQ(v.size(), static_cast<size_t>(s.read_async(b).get()));

    for (size_t i = 0; i < v.size(); ++i)
    {
        ASSERT_EQ(v[i], b[i]);
    }
}
