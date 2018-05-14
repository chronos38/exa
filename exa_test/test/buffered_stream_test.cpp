#include <pch.h>
#include <exa/buffered_stream.hpp>
#include <exa/memory_stream.hpp>

using namespace exa;
using namespace std::literals::chrono_literals;

namespace
{
    auto create_stream(std::shared_ptr<memory_stream> s = std::make_shared<memory_stream>(), std::streamsize bs = 4096)
    {
        return std::make_shared<buffered_stream>(s, bs);
    }

    auto create_data(size_t size = 1000)
    {
        std::vector<uint8_t> v(size, 0);

        for (size_t i = 0; i < size; ++i)
        {
            v[i] = static_cast<uint8_t>(i);
        }

        return v;
    }

    struct test_stream : public memory_stream
    {
        bool seekable = true;
        bool writable = true;
        bool readable = true;

        virtual bool can_seek() const override
        {
            return seekable;
        }

        virtual bool can_write() const override
        {
            return writable;
        }

        virtual bool can_read() const override
        {
            return readable;
        }
    };

    struct concurrent_stream : public memory_stream
    {
        virtual void write(gsl::span<const uint8_t> b) override
        {
            exa::lock(*this, [&] { memory_stream::write(b); });
        }

        virtual std::streamsize read(gsl::span<uint8_t> b) override
        {
            std::streamsize r = 0;
            exa::lock(*this, [&] { r = memory_stream::read(b); });
            return r;
        }
    };

    struct throw_stream : public memory_stream
    {
        virtual void write(gsl::span<const uint8_t> b) override
        {
            throw std::runtime_error("Operation not allowed.");
        }

        virtual std::streamsize read(gsl::span<uint8_t> b) override
        {
            throw std::runtime_error("Operation not allowed.");
        }

        virtual void flush() override
        {
            throw std::runtime_error("Operation not allowed.");
        }

        virtual std::streamoff seek(std::streamoff offset, seek_origin origin) override
        {
            throw std::runtime_error("Operation not allowed.");
        }
    };
}

TEST(buffered_stream_test, concurrent_operation_are_serialized)
{
    auto v = create_data();
    auto inner = std::make_shared<concurrent_stream>();
    auto s = create_stream(inner, 1);

    std::array<std::future<void>, 4> tasks;

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        tasks[i] = s->write_async(gsl::span<uint8_t>(v.data() + 250 * i, 250));
    }
    for (auto& t : tasks)
    {
        ASSERT_NO_THROW(t.get());
    }

    s->position(0);

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        ASSERT_EQ(i, s->read_byte());
    }
}

TEST(buffered_stream_test, underlying_stream_throws_exception)
{
    auto s = create_stream(std::make_shared<throw_stream>());
    std::vector<uint8_t> v1(1);
    std::vector<uint8_t> v2(10000);

    ASSERT_THROW(s->read_async(v1).get(), std::runtime_error);
    ASSERT_THROW(s->write_async(v2).get(), std::runtime_error);
    s->write_byte(1);
    ASSERT_THROW(s->flush_async().get(), std::runtime_error);
}

TEST(buffered_stream_test, copy_to_requires_flushing_of_writes)
{
    for (auto async : {false, true})
    {
        auto s = create_stream();
        auto v = create_data();
        auto d = std::make_shared<memory_stream>();

        s->write(v);
        s->position(0);

        v[0] = 42;
        s->write_byte(42);
        d->write_byte(42);

        if (async)
        {
            s->copy_to_async(d).get();
        }
        else
        {
            s->copy_to(d);
        }

        auto r = d->to_array();

        for (size_t i = 0; i < v.size(); ++i)
        {
            ASSERT_EQ(v[i], r[i]);
        }
    }
}

TEST(buffered_stream_test, copy_to_read_before_copy_copies_all_data)
{
    std::vector<std::pair<bool, bool>> data = {{false, false}, {false, true}, {true, false}, {true, true}};

    for (auto input : data)
    {
        auto v = create_data();
        auto ts = std::make_shared<test_stream>();

        ts->write(v);
        ts->position(0);
        ts->seekable = input.first;

        auto src = create_stream(ts, 100);
        src->read_byte();
        auto dst = std::make_shared<memory_stream>();

        if (input.second)
        {
            src->copy_to_async(dst).get();
        }
        else
        {
            src->copy_to(dst);
        }

        std::vector<uint8_t> expected(v.size() - 1);
        std::copy(std::next(std::begin(v), 1), std::end(v), std::begin(expected));
        auto array = dst->to_array();

        for (size_t i = 0; i < expected.size(); ++i)
        {
            ASSERT_EQ(expected[i], array[i]);
        }
    }
}
