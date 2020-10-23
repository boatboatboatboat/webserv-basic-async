//
// Created by Djevayo Pattij on 10/23/20.
//

#include "StringStream.hpp"

auto utils::StringStream::str() -> std::string
{
    return std::move(_buffer);
}

auto utils::StringStream::operator<<(std::string_view view) -> utils::StringStream&
{
    _buffer.append(view);
    return *this;
}

auto utils::StringStream::operator<<(size_t view) -> utils::StringStream&
{
    _buffer.append(std::to_string(view));
    return *this;
}

auto utils::StringStream::operator<<(ssize_t view) -> utils::StringStream&
{
    _buffer.append(std::to_string(view));
    return *this;
}

utils::StringStream::StringStream(std::string_view view)
    : _buffer(view)
{
}

auto utils::StringStream::operator<<(int view) -> utils::StringStream&
{
    _buffer.append(std::to_string(view));
    return *this;
}
