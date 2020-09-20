//
// Created by boat on 9/18/20.
//

#ifndef WEBSERV_OPTIONAL_HPP
#define WEBSERV_OPTIONAL_HPP

#include <algorithm>

namespace option {

struct nullopt_t {};
constexpr nullopt_t nullopt {};

template <typename T>
class optional {
private:
    bool some;
    union {
        T t;
    };

public:
    optional()
        : some(false)
    {
    }
    optional(const nullopt_t)
        : some(false)
    {
    }
    ~optional()
    {
        reset();
    }
    static auto none() -> optional { return optional(); }
    optional(T&& t)
        : some(true)
        , t(std::move(t))
    {
    }
    void reset() noexcept
    {
        if (some)
            t.~T();
        some = false;
    }
    explicit operator bool() const { return has_value(); }
    [[nodiscard]] auto has_value() const -> bool { return some; }
    [[nodiscard]] auto value() -> T&
    {
        if (!some)
            throw std::runtime_error("bad optional access");
        return t;
    }
    [[nodiscard]] auto value() const -> T const&
    {
        if (!some)
            throw std::runtime_error("bad optional access");
        return t;
    }
    [[nodiscard]] auto value_or(T&& e) -> T&
    {
        return some ? t : e;
    }
    [[nodiscard]] auto value_or(T&& e) const -> T const&
    {
        return some ? t : e;
    }
    constexpr auto operator->() const -> const T*
    {
        return &value();
    }
    constexpr auto operator->() -> T*
    {
        return &value();
    }
    constexpr auto operator*() const& -> const T&
    {
        return value();
    }
    constexpr auto operator*() & -> T&
    {
        return value();
    }
    constexpr auto operator*() const&& -> const T&&
    {
        if (!some)
            throw std::runtime_error("bad optional access");
        return std::move(t);
    }
    constexpr auto operator*() && -> T&&
    {
        if (!some)
            throw std::runtime_error("bad optional access");
        some = false;
        return std::move(t);
    }
    auto operator=(const nullopt_t) noexcept -> optional&
    {
        reset();
        return *this;
    }
    constexpr auto operator=(const optional& other) -> optional&
    {
        reset();
        t = other.t;
        some = other.some;
        return *this;
    }
    constexpr auto operator=(optional&& other) noexcept -> optional&
    {
        reset();
        some = other.some;
        t = std::move(other.t);
        other.reset();
        return *this;
    }
    template <class U = T>
    auto operator=(U&& value) -> optional&
    {
        reset();
        some = true;
        t = std::forward<U>(value);
        return *this;
    }
    template <class U>
    auto operator=(const optional<U>& other) -> optional&
    {
        reset();
        some = other.some;
        t = other.t;
        return *this;
    }
    template <class U>
    auto operator=(optional<U>&& other) -> optional&
    {
        reset();
        some = other.some;
        t = std::move(other.t);
        other.reset();
        return *this;
    }
};

}

#endif //WEBSERV_OPTIONAL_HPP
