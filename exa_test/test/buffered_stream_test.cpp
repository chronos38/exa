#include <pch.h>
#include <exa/buffered_stream.hpp>
#include <exa/memory_stream.hpp>
#include <exa/concepts.hpp>

using namespace exa;
using namespace testing;
using namespace std::chrono_literals;

namespace
{
    auto create_stream(std::unique_ptr<stream>&& s = std::make_unique<memory_stream>(), std::streamsize bs = 4096)
    {
        return std::make_unique<buffered_stream>(s.release(), bs);
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

    struct concurrent_stream : public memory_stream, public lockable<std::mutex>
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

    struct tracking_stream : public stream
    {
        virtual ~tracking_stream() = default;

        mutable struct
        {
            int can_read = 0;
            int can_seek = 0;
            int can_write = 0;
            int size = 0;
            int position = 0;
            int flush = 0;
            int read = 0;
            int seek = 0;
            int write = 0;
        } called;

        std::shared_ptr<stream> stream;

        tracking_stream(std::shared_ptr<exa::stream> s)
        {
            stream = s;
        }

        // Inherited via stream
        virtual bool can_read() const override
        {
            called.can_read += 1;
            return stream->can_read();
        }

        virtual bool can_seek() const override
        {
            called.can_seek += 1;
            return stream->can_seek();
        }

        virtual bool can_write() const override
        {
            called.can_write += 1;
            return stream->can_write();
        }

        virtual std::streamsize size() const override
        {
            called.size += 1;
            return stream->size();
        }

        virtual void size(std::streamsize value) override
        {
            called.size += 1;
            stream->size();
        }

        virtual std::streamoff position() const override
        {
            called.position += 1;
            return stream->position();
        }

        virtual void position(std::streamoff value) override
        {
            called.position += 1;
            stream->position();
        }

        virtual void flush() override
        {
            called.flush += 1;
            stream->flush();
        }

        virtual std::streamsize read(gsl::span<uint8_t> buffer) override
        {
            called.read += 1;
            return stream->read(buffer);
        }

        virtual std::streamoff seek(std::streamoff offset, seek_origin origin) override
        {
            called.seek += 1;
            return stream->seek(offset, origin);
        }

        virtual void write(gsl::span<const uint8_t> buffer) override
        {
            called.write += 1;
            stream->write(buffer);
        }
    };
}

TEST(buffered_stream_test, concurrent_operations_are_serialized)
{
    auto v = create_data();
    auto s = create_stream(std::make_unique<concurrent_stream>(), 1);

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
        auto b = s->read_byte();
        ASSERT_TRUE(b == i || b == static_cast<uint8_t>(i + 250) || b == static_cast<uint8_t>(i + 500) ||
                    b == static_cast<uint8_t>(i + 750));
    }
}

TEST(buffered_stream_test, underlying_stream_throws_exception)
{
    auto s = create_stream(std::make_unique<throw_stream>());
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
        auto d = std::make_unique<memory_stream>();

        s->write(v);
        s->position(0);

        v[0] = 42;
        s->write_byte(42);
        d->write_byte(42);

        if (async)
        {
            s->copy_to_async(d.get()).get();
        }
        else
        {
            s->copy_to(d.get());
        }

        auto r = d->to_array();
        ASSERT_THAT(r, ContainerEq(v));
    }
}

TEST(buffered_stream_test, copy_to_read_before_copy_copies_all_data)
{
    std::vector<std::pair<bool, bool>> data = {{false, false}, {false, true}, {true, false}, {true, true}};

    for (auto input : data)
    {
        auto v = create_data();
        auto ts = std::make_unique<test_stream>();

        ts->write(v);
        ts->position(0);
        ts->seekable = input.first;

        auto src = create_stream(move(ts), 100);
        src->read_byte();
        auto dst = std::make_unique<memory_stream>();

        if (input.second)
        {
            src->copy_to_async(dst.get()).get();
        }
        else
        {
            src->copy_to(dst.get());
        }

        std::vector<uint8_t> expected(v.size() - 1);
        std::copy(std::next(std::begin(v), 1), std::end(v), std::begin(expected));

        auto array = dst->to_array();
        ASSERT_THAT(array, ContainerEq(expected));
    }
}

TEST(buffered_stream_test, should_not_flush_underyling_stream_if_readonly)
{
    for (auto can_seek : {true, false})
    {
        auto underlying = std::make_shared<test_stream>();
        underlying->write(create_data(123));
        underlying->position(0);
        underlying->seekable = can_seek;
        underlying->writable = false;

        auto s = create_stream(std::make_unique<tracking_stream>(underlying));
        auto tracker = dynamic_cast<tracking_stream*>(s->underlying_stream());

        s->read_byte();
        s->flush();
        ASSERT_THAT(tracker->called.flush, Eq(0));

        s->flush_async().get();
        ASSERT_THAT(tracker->called.flush, Eq(0));
    }
}

TEST(buffered_stream_test, should_always_flush_underlying_stream_if_writable)
{
    std::vector<std::pair<bool, bool>> v = {{true, true}, {true, false}, {false, true}, {false, false}};

    for (auto input : v)
    {
        auto underlying = std::make_shared<test_stream>();
        underlying->write(create_data(123));
        underlying->position(0);
        underlying->readable = input.first;
        underlying->seekable = input.second;
        underlying->writable = true;

        auto s = create_stream(std::make_unique<tracking_stream>(underlying));
        auto tracker = dynamic_cast<tracking_stream*>(s->underlying_stream());

        s->flush();
        ASSERT_THAT(tracker->called.flush, Eq(1));

        s->flush_async().get();
        ASSERT_THAT(tracker->called.flush, Eq(2));

        s->write_byte(42);
        s->flush();
        ASSERT_THAT(tracker->called.flush, Eq(3));

        s->flush_async().get();
        ASSERT_THAT(tracker->called.flush, Eq(4));
    }
}
