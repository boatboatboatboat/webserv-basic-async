//
// Created by boat on 10/1/20.
//

#include "IoResult.hpp"

namespace ioruntime {

IoResult::IoResult(ssize_t posix_io_result)
{
    if (posix_io_result < 0) {
        _tag = Error;
    } else if (posix_io_result == 0) {
        _tag = Eof;
        _bytes_read = 0;
    } else {
        _tag = Success;
        _bytes_read = posix_io_result;
    }
}

IoResult::IoResult(Tag tag)
    : _tag(tag)
    , _bytes_read(0)
{
}

IoResult::IoResult(Tag tag, size_t bytes_read)
    : _tag(tag)
    , _bytes_read(bytes_read)
{
}

auto IoResult::is_error() const -> bool
{
    return _tag == Error;
}

auto IoResult::is_eof() const -> bool
{
    return _tag == Eof;
}

auto IoResult::is_success() const -> bool
{
    return _tag == Success;
}

auto IoResult::get_bytes_read() const -> size_t
{
    return _bytes_read;
}

auto IoResult::eof() -> IoResult
{
    return IoResult(Eof, 0);
}

auto IoResult::error() -> IoResult
{
    return IoResult(Error);
}

auto IoResult::success(size_t bytes_read) -> IoResult
{
    return IoResult(Success, bytes_read);
}

}