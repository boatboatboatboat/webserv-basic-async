//
// Created by boat on 9/18/20.
//

#ifndef WEBSERV_OPTIONAL_HPP
#define WEBSERV_OPTIONAL_HPP

#include "../utils/mem_copy.hpp"
#include <algorithm>

namespace option {

struct nullopt_t {
    explicit constexpr nullopt_t(int) { }
};
constexpr nullopt_t nullopt(0);

// NOT std::optional!
// optional datatype
template <typename T>
class optional {
private:
    bool some;
    union {
        T t;
    };

    // We need a custom setter for ``t``,
    // because (move) assignment constructors assume ``t`` is initialized.
    // However we disabled ctor on ``t`` (so ``t`` can be uninitialized).
    //
    // We can however call the regular move/copy constructors by using a
    // placement new.

    // Set the value of t, checking for uninitialized t
    constexpr void t_set(const T& a)
    {
        if (some) {
            t = a;
        } else {
            new (&t) T(a);
        }
    }
    // Set the value of t, checking for uninitialized t
    constexpr void t_move(T&& a)
    {
        if (some) {
            // we use a dtor+ctor instead of a move assignment,
            // because for some reason C++ can't move-assign classes
            //  with const-reference members :-)
            t.~T();
            new (&t) T(std::forward<T>(a));
        } else {
            new (&t) T(std::forward<T>(a));
        }
    }

public:
    constexpr optional()
        : some(false)
    {
    }
    constexpr optional(const nullopt_t)
        : some(false)
    {
    }
    constexpr optional(optional const& other)
    {
        some = false;
        if (other.has_value())
            t_set(other.value());
        some = other.some;
    }
    constexpr optional(optional&& other)
    {
        some = false;
        if (other.has_value())
            t_move(std::move(other.value()));
        some = other.some;
        other.reset();
    }
    template <typename U>
    optional(optional<U> const& other)
    {
        some = false;
        if (other.has_value())
            t_set(other.value());
        some = other.has_value();
    }
    template <typename U>
    optional(optional<U>&& other)
    {
        some = false;
        if (other.has_value())
            t_move(std::move(other.value()));
        some = other.has_value();
        other.reset();
    }
    // FIXME: if U is deduced to a optional<T>, and T is a bool, then C++ ignores explicit bool rules and casts it to a bool
    template <typename U = T>
    optional(U&& value)
        : some(true)
        , t(std::forward<U>(value))
    {
    }
    ~optional()
    {
        reset();
    }
    static auto none() -> optional { return optional(); }
    void reset() noexcept
    {
        if (some)
            t.~T();
        some = false;
    }
    // explicit operator bool() const { return has_value(); }
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
        if (other.has_value())
            t_set(other.value());
        some = other.has_value();
        return *this;
    }
    constexpr auto operator=(optional&& other) noexcept -> optional&
    {
        reset();
        if (other.has_value())
            t_move(std::move(other.value()));
        some = other.has_value();
        other.reset();
        return *this;
    }
    template <class U>
    auto operator=(const optional<U>& other) -> optional&
    {
        reset();
        if (other.has_value())
            t_set(other.value());
        some = other.has_value();
        return *this;
    }
    template <class U>
    auto operator=(optional<U>&& other) -> optional&
    {
        reset();
        if (other.has_value())
            t_move(std::move(other.value()));
        some = other.has_value();
        other.reset();
        return *this;
    }
    template <class U = T>
    auto operator=(U&& value) -> optional&
    {
        reset();
        t_move(std::move(value));
        some = true;
        return *this;
    }
};

//template <typename T>
//constexpr auto make_optional(T&& v) -> optional<T>
//{
//    return optional<T>(std::forward<T>(v));
//}
template <typename T, typename... Args>
constexpr auto make_optional(Args&&... args) -> optional<T>
{
    return optional<T>(std::forward<Args>(args)...);
}

}

#endif //WEBSERV_OPTIONAL_HPP
