//
// Created by boat on 9/27/20.
//

#ifndef WEBSERV_ARGS_ARGS_HPP
#define WEBSERV_ARGS_ARGS_HPP

#include <string>
using std::string_view;

namespace args {

class Args;

class ArgsIterator {
    friend class Args;

public:
    auto operator++() -> ArgsIterator&;
    auto operator--() -> ArgsIterator&;
    auto operator+=(ssize_t n) -> ArgsIterator&;
    auto operator-=(ssize_t n) -> ArgsIterator&;
    auto operator*() -> string_view;
    auto operator==(ArgsIterator const& other) const -> bool;
    auto operator!=(ArgsIterator const& other) const -> bool;

private:
    ArgsIterator(const char** argv, int ca);

    const char** _argv;
    int _current_arg;
};

class Args {
public:
    Args(int argc, const char** argv);

    [[nodiscard]] auto begin() const noexcept -> ArgsIterator;
    [[nodiscard]] auto end() const noexcept -> ArgsIterator;
    [[nodiscard]] auto count() const noexcept -> int;
private:
    const char** _argv;
    int _argc;
};

}

#endif //WEBSERV_ARGS_ARGS_HPP
