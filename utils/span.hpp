//
// Created by boat on 13-09-20.
//

#ifndef WEBSERV_UTILS_SPAN_HPP
#define WEBSERV_UTILS_SPAN_HPP

#include <cstdio>

namespace utils {

template <typename T>
class span {
    T* m_ptr;
    size_t m_len;

public:
    span(T* ptr, std::size_t len) noexcept
        : m_ptr { ptr }
        , m_len { len }
    {
    }

    T& operator[](ssize_t i) noexcept
    {
        return *m_ptr[i];
    }

    T const& operator[](ssize_t i) const noexcept
    {
        return *m_ptr[i];
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return m_len;
    }

    T* begin() noexcept
    {
        return m_ptr;
    }

    T* end() noexcept
    {
        return m_ptr + m_len;
    }

    T* rbegin() noexcept
    {
        return m_ptr + m_len - 1;
    }

    T* rend() noexcept
    {
        return m_ptr - 1;
    }

    auto cbegin() const noexcept -> T const*
    {
        return m_ptr;
    }

    auto cend() const noexcept -> T const*
    {
        return m_ptr + m_len;
    }

    [[nodiscard]] auto front() -> T&
    {
        return *m_ptr;
    }

    [[nodiscard]] auto back() -> T&
    {
        return m_ptr[m_len];
    }

    [[nodiscard]] auto data() -> T*
    {
        return m_ptr;
    }

    [[nodiscard]] auto size_bytes() const -> std::size_t
    {
        return m_len * sizeof(T);
    }

    [[nodiscard]] auto empty() const -> std::size_t
    {
        return m_len == 0;
    }

    [[nodiscard]] auto first(size_t count) const -> span<T>
    {
        return span<T>(m_ptr, count > size() ? size() : count);
    }

    auto remove_prefix_inplace(size_t count)
    {
        *this = last(size() - count);
    }

    auto remove_suffix_inplace(size_t count)
    {
        *this = first(size() - count);
    }

    inline auto remove_prefix(size_t count)
    {
        return last(size() - count);
    }

    inline auto remove_suffix(size_t count)
    {
        return first(size() - count);
    }

    [[nodiscard]] auto last(size_t count) const -> span<T>
    {
        return span<T>(m_ptr + (count > m_len ? m_len : m_len - count), (count > m_len) ? m_len : count);
    }
};

}

#endif //WEBSERV_UTILS_SPAN_HPP
