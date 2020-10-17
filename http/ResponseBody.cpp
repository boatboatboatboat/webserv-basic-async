//
// Created by Djevayo Pattij on 10/2/20.
//

#include "ResponseBody.hpp"

namespace http {

using std::move;

ResponseBody::ResponseBody(ResponseBody&& other) noexcept
{
    if (other._tag == ReaderTag) {
        new (&_reader) BoxPtr<IAsyncRead>(move(other._reader));
    } else if (other._tag == CgiTag) {
        new (&_cgi) Cgi(move(other._cgi));
    }
    _tag = other._tag;
}

auto ResponseBody::operator=(ResponseBody&& other) noexcept -> ResponseBody&
{
    if (_tag == ReaderTag) {
        _reader.~BoxPtr();
    } else if (_tag == CgiTag) {
        _cgi.~Cgi();
    }
    if (other._tag == ReaderTag) {
        new (&_reader) BoxPtr<IAsyncRead>(move(other._reader));
    } else if (other._tag == CgiTag) {
        new (&_cgi) Cgi(move(other._cgi));
    }
    _tag = other._tag;
    return *this;
}

ResponseBody::~ResponseBody()
{
    if (_tag == ReaderTag) {
        _reader.~BoxPtr();
    } else if (_tag == CgiTag) {
        _cgi.~Cgi();
    }
}

ResponseBody::ResponseBody(BoxPtr<ioruntime::IAsyncRead>&& reader)
{
    _tag = ReaderTag;
    new (&_reader) BoxPtr<IAsyncRead>(move(reader));
}

ResponseBody::ResponseBody(Cgi&& cgi)
{
    _tag = CgiTag;
    new (&_cgi) Cgi(move(cgi));
}

auto ResponseBody::is_reader() const -> bool
{
    return _tag == ReaderTag;
}

auto ResponseBody::is_cgi() const -> bool
{
    return _tag == CgiTag;
}

auto ResponseBody::get_cgi() -> Cgi&
{
    return _cgi;
}

auto ResponseBody::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<ioruntime::IoResult>
{
    if (_tag == ReaderTag) {
        return _reader->poll_read(buffer, move(waker));
    } else if (_tag == CgiTag) {
        return _cgi.poll_read(buffer, move(waker));
    } else {
        throw std::logic_error("Unreachable");
    }
}

}