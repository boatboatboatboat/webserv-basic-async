//
// Created by Djevayo Pattij on 10/2/20.
//

#include "Body.hpp"

namespace http {

using std::move;

Body::Body(Body&& other) noexcept
{
    if (other._tag == ReaderTag) {
        new (&_reader) BoxPtr<IAsyncRead>(move(other._reader));
    } else if (other._tag == CgiTag) {
        new (&_cgi) Cgi(move(_cgi));
    }
    _tag = other._tag;
}

auto Body::operator=(Body&& other) noexcept -> Body&
{
    if (_tag == ReaderTag) {
        _reader.~BoxPtr();
    } else if (_tag == CgiTag) {
        _cgi.~Cgi();
    }
    if (other._tag == ReaderTag) {
        new (&_reader) BoxPtr<IAsyncRead>(move(other._reader));
    } else if (other._tag == CgiTag) {
        new (&_cgi) Cgi(move(_cgi));
    }
    _tag = other._tag;
}

Body::~Body()
{
    if (_tag == ReaderTag) {
        _reader.~BoxPtr();
    } else if (_tag == CgiTag) {
        _cgi.~Cgi();
    }
}

Body::Body(BoxPtr<ioruntime::IAsyncRead>&& reader)
{
    _tag = ReaderTag;
    new (&_reader) BoxPtr<IAsyncRead>(move(reader));
}

Body::Body(Cgi&& cgi)
{
    _tag = CgiTag;
    new (&_cgi) Cgi(move(cgi));
}

auto Body::is_reader() const -> bool
{
    return _tag == ReaderTag;
}

auto Body::is_cgi() const -> bool
{
    return _tag == CgiTag;
}

auto Body::get_cgi() -> Cgi&
{
    return _cgi;
}

auto Body::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<ioruntime::IoResult>
{
    if (_tag == ReaderTag) {
        return _reader->poll_read(buffer, move(waker));
    } else if (_tag == CgiTag) {
        return _cgi.poll_read(buffer, move(waker));
    }
}

}