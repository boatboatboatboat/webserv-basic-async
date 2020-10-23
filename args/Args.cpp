//
// Created by boat on 9/27/20.
//

#include "Args.hpp"
#include "../utils/utils.hpp"

namespace args {

Args::Args(int argc, const char** argv)
    : _argv(argv)
    , _argc(argc)
{
}

auto Args::begin() const noexcept -> ArgsIterator {
    return ArgsIterator(_argv, 0);
}

auto Args::end() const noexcept -> ArgsIterator
{
    return ArgsIterator(_argv, _argc);
}

auto Args::count() const noexcept -> int
{
    return _argc;
}

ArgsIterator::ArgsIterator(const char** argv, int ca)
    : _argv(argv)
    , _current_arg(ca)
{
}

auto ArgsIterator::operator++() -> ArgsIterator&
{
    *this += 1;
    return *this;
}

auto ArgsIterator::operator--() -> ArgsIterator&
{
    *this -= 1;
    return *this;
}

auto ArgsIterator::operator+=(ssize_t n) -> ArgsIterator&
{
    _current_arg += n;
    return *this;
}

auto ArgsIterator::operator-=(ssize_t n) -> ArgsIterator&
{
    _current_arg -= n;
    return * this;
}

auto ArgsIterator::operator*() -> string_view
{
    return string_view(_argv[_current_arg]);
}

auto ArgsIterator::operator==(ArgsIterator const& other) const -> bool
{
    return _current_arg == other._current_arg;
}

auto ArgsIterator::operator!=(ArgsIterator const& other) const -> bool
{
    return !(*this == other);
}

}