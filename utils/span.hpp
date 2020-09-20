//
// Created by boat on 13-09-20.
//

#ifndef WEBSERV_UTILS_SPAN_HPP
#define WEBSERV_UTILS_SPAN_HPP

#include <cstdio>

namespace utils {

template<typename T>
class span {
    T* m_ptr;
    size_t m_len;

public:
    span(T* ptr, std::size_t len) noexcept
        : m_ptr {ptr}, m_len {len}
    {}

    T& operator[](ssize_t i) noexcept {
        return *m_ptr[i];
    }

    T const& operator[](ssize_t i) const noexcept {
        return *m_ptr[i];
    }

    [[nodiscard]] size_t size() const noexcept {
        return m_len;
    }

    T* begin() noexcept {
        return m_ptr;
    }

    T* end() noexcept {
        return m_ptr + m_len;
    }

    T* rbegin() noexcept {
        return m_ptr + m_len - 1;
    }

    T* rend() noexcept {
        return m_ptr - 1;
    }
};

}

#endif //WEBSERV_UTILS_SPAN_HPP
