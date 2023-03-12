// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cassert>
#include <memory>
#include <mutex>
#include <sstream>
#include <streambuf>
#include <string>

namespace fauxstd {

template <typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT>>
class basic_syncbuf : public std::basic_streambuf<CharT, Traits>
{
public:
    using char_type      = CharT;
    using traits_type    = Traits;
    using int_type       = typename Traits::int_type;
    using pos_type       = typename Traits::pos_type;
    using off_type       = typename Traits::off_type;
    using allocator_type = Allocator;
    using streambuf_type = std::basic_streambuf<CharT, Traits>;

    static_assert(std::is_same<char_type, typename traits_type::char_type>::value, "These need to be the same");

    basic_syncbuf()
    : basic_syncbuf(nullptr)
    {
    }

    explicit basic_syncbuf(streambuf_type* obuf)
    : basic_syncbuf(obuf, Allocator())
    {
    }

    basic_syncbuf(streambuf_type* obuf, const Allocator& a)
    : m_emit_on_sync(false)
    , m_need_pubsync(false)
    , m_streambuf(obuf)
#if __cplusplus >= 202002L
    // C++20 added stateful allocator support to std::basic_stringbuf. Before C++20, this constructor simply ignores the
    // allocator instance (the allocator type is still used).
    , m_buffer(a)
#else
    , m_buffer()
#endif
    , m_allocator(a)
    {
    }

    basic_syncbuf(basic_syncbuf&& other)
    : m_emit_on_sync(other.m_emit_on_sync)
    , m_need_pubsync(other.m_need_pubsync)
    , m_streambuf(other.m_streambuf)
    , m_allocator(std::move(other.m_allocator))
    , m_buffer(std::move(other.m_buffer))
    {
        m_streambuf = nullptr;
    }

    ~basic_syncbuf()
    {
        try {
            emit();
        } catch (...) {
        }
    }

    bool emit()
    {
        if (m_streambuf == nullptr) {
            return false;
        }

        auto s = std::move(m_buffer).str();

        const auto size = s.size();
        if (size == 0) {
            m_need_pubsync = false;
            return true;
        }

        std::lock_guard<std::mutex> stream_lock(s_stream_mutexes[get_mutex_index(m_streambuf)]);

        const auto n = m_streambuf->sputn(s.data(), size);
        if (n != size) {
            s.erase(0, n);
            m_buffer.str(std::move(s));
            return false;
        }

        if (m_need_pubsync) {
            m_need_pubsync = false;
            return m_streambuf->pubsync() == 0;
        }
        return true;
    }

    streambuf_type* get_wrapped() const noexcept
    {
        return this->m_streambuf;
    }

    allocator_type get_allocator() const noexcept
    {
        return m_allocator;
    }

    void set_emit_on_sync(bool b) noexcept
    {
        m_emit_on_sync = b;
    }

    void swap(basic_syncbuf& other)
    {
        using std::swap; // Allow ADL
        swap(m_emit_on_sync, other.m_emit_on_sync);
        swap(m_need_pubsync, other.m_need_pubsync);
        swap(m_streambuf, other.m_streambuf);
        swap(m_buffer, other.m_buffer);

        // TODO: may need better logic here: e.g. is always same checks...
        swap(m_allocator, other.m_allocator);
    }

protected:
    int sync() override
    {
        m_need_pubsync = true;
        if (m_emit_on_sync && !emit()) {
            return -1;
        }
        return 0;
    }

    int_type overflow(int_type c) override
    {
        const int_type eof = traits_type::eof();
        if (!traits_type::eq_int_type(c, eof)) {
            return m_buffer.sputc(c);
        }
        return eof;
    }

    std::streamsize xsputn(const char_type* s, std::streamsize n) override
    {
        return m_buffer.sputn(s, n);
    }

private:
    static auto get_pointer_hash(void* p) noexcept
    {
        // std::hash<void*> is well within its rights to return the pointer unchanged. With pointer alignment, taking
        // the mod of this value against small values probably results in 0 each time. Therefore, we provide a separate
        // hash function (https://nullprogram.com/blog/2018/07/31/).
        auto x = reinterpret_cast<std::uintptr_t>(p);
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9U;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebU;
        x ^= x >> 31;
        return x;
    }

    static std::size_t get_mutex_index(void* p) noexcept
    {
        return get_pointer_hash(p) % std::tuple_size<map_type>::value;
    }

    using buffer_type = std::basic_stringbuf<char_type, traits_type, allocator_type>;
    using map_type    = std::array<std::mutex, 16>;

    static map_type s_stream_mutexes;

    bool            m_emit_on_sync;
    bool            m_need_pubsync;
    streambuf_type* m_streambuf;
    buffer_type     m_buffer;

#if __has_cpp_attribute(no_unique_address)
    [[no_unique_address]] allocator_type m_allocator;
#else
    allocator_type m_allocator;
#endif
};

template <typename CharT, typename Traits, typename Allocator>
typename basic_syncbuf<CharT, Traits, Allocator>::map_type basic_syncbuf<CharT, Traits, Allocator>::s_stream_mutexes;

template <typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT>>
class basic_osyncstream : public std::basic_ostream<CharT, Traits>
{
    using ostream_type = std::basic_ostream<CharT, Traits>;

public:
    using char_type      = CharT;
    using traits_type    = Traits;
    using int_type       = typename Traits::int_type;
    using pos_type       = typename Traits::pos_type;
    using off_type       = typename Traits::off_type;
    using allocator_type = Allocator;
    using streambuf_type = std::basic_streambuf<CharT, Traits>;
    using syncbuf_type   = basic_syncbuf<CharT, Traits>;

    static_assert(std::is_same<char_type, typename traits_type::char_type>::value, "These need to be the same");

    basic_osyncstream(streambuf_type* buf, const Allocator& a)
    : m_syncbuf(buf, a)
    {
        this->init(std::addressof(m_syncbuf));
    }

    explicit basic_osyncstream(streambuf_type* buf)
    : m_syncbuf(buf)
    {
        this->init(std::addressof(m_syncbuf));
    }

    basic_osyncstream(std::basic_ostream<CharT, Traits>& os, const Allocator& a)
    : m_syncbuf(os.rdbuf(), a)
    {
        this->init(std::addressof(m_syncbuf));
    }

    explicit basic_osyncstream(std::basic_ostream<CharT, Traits>& os)
    : m_syncbuf(os.rdbuf())
    {
        this->init(std::addressof(m_syncbuf));
    }

    basic_osyncstream(basic_osyncstream&& other) noexcept
    : ostream_type(std::move(other))
    , m_syncbuf(std::move(other.m_syncbuf))
    {
        ostream_type::set_rdbuf(std::addressof(m_syncbuf));
    }

    ~basic_osyncstream()                                       = default;
    basic_osyncstream& operator=(basic_osyncstream&&) noexcept = default;

    syncbuf_type* rdbuf() const noexcept { return const_cast<syncbuf_type*>(std::addressof(m_syncbuf)); }

    streambuf_type* get_wrapped() const noexcept { return m_syncbuf.get_wrapped(); }

    void emit()
    {
        if (!m_syncbuf.emit()) {
            this->setstate(std::ios_base::failbit);
        }
    }

private:
    syncbuf_type m_syncbuf;
};

using syncbuf      = basic_syncbuf<char>;
using wsyncbuf     = basic_syncbuf<wchar_t>;
using osyncstream  = basic_osyncstream<char>;
using wosyncstream = basic_osyncstream<wchar_t>;

} // namespace fauxstd


