//
// Created by boat on 11-10-20.
//

#include "IncomingBody.hpp"

using std::move;

auto http::IncomingBody::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_body_type) {
        case VectorBody: {
            if (!_spr.has_value()) {
                _spr = ioruntime::SpanReader(span(_vector->data(), _size));
            }
            auto res = _spr->poll_read(buffer, move(waker));
            if (res.is_ready()) {
                TRACEPRINT("VectorRead " << res.get().get_bytes_read());
            }
            return res;
        } break;
        case TemporaryBody: {
            if (_should_rewind) {
                _should_rewind = false;
                lseek(_temp->get_descriptor(), 0, SEEK_SET);
            }
            auto res = _temp->poll_read(buffer, move(waker));
            if (res.is_ready()) {
                TRACEPRINT("FileRead " << res.get().get_bytes_read());
            }
            return res;
        } break;
    }
}

auto http::IncomingBody::poll_write(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    _should_rewind = true;
    switch (_body_type) {
        case VectorBody: {
            if (_converting) {
                auto pr = _icf->poll(Waker(waker));
                if (pr.is_ready()) {
                    _converting = false;
                    _body_type = TemporaryBody;
                    _vector.reset();
                    _spr.reset();
                    _icf.reset();
                    return poll_write(buffer, move(waker));
                }
                return PollResult<IoResult>::pending();
            } else {
                if (buffer.size() + _vector->size() >= REQUEST_BODY_SWITCH_TEMPORARY_SIZE) {
                    _converting = true;
                    _spr = ioruntime::SpanReader(span(_vector->data(), _vector->size()));
                    _temp = File::temporary();
                    _icf = ioruntime::RefIoCopyFuture(*_spr, *_temp);
                    return poll_write(buffer, std::move(waker));
                } else {
                    _vector->insert(_vector->end(), buffer.begin(), buffer.end());
                    _size += buffer.size();
                    waker();
                    return PollResult<IoResult>::ready(buffer.size());
                }
            }
        } break;
        case TemporaryBody: {
            auto pr = _temp->poll_write(buffer, std::move(waker));
            if (pr.is_ready()) {
                auto const& res = pr.get();
                if (!res.is_eof()) {
                    _size += res.get_bytes_read();
                }
            }
            return pr;
        } break;
    }
}

auto http::IncomingBody::size() const -> size_t
{
    return _size;
}

http::IncomingBody::IncomingBody(http::IncomingBody&& other) noexcept
    : _body_type(other._body_type)
    , _size(other._size)
    , _converting(other._converting)
    , _vector(move(other._vector))
    , _should_rewind(true)
{
    if (other._temp.has_value()) {
        _temp = move(other._temp);
    }
    if (other._spr.has_value()) {
        auto newspan = span(_vector->data(), _vector->size());
        auto spr = ioruntime::SpanReader(newspan);
        spr.discard_first(other._spr->get_bytes_read());
        _spr = move(spr);
    }
    if (other._icf.has_value()) {
        _icf = ioruntime::RefIoCopyFuture(*_spr, *_temp);
    }
}

auto http::IncomingBody::operator=(http::IncomingBody&& other) noexcept -> http::IncomingBody&
{
    if (&other == this)
        return *this;
    _body_type = other._body_type;
    _size = other._size;
    _vector = move(other._vector);
    _should_rewind = other._should_rewind;
    if (other._temp.has_value()) {
        _temp = move(*other._temp);
    }
    if (other._spr.has_value()) {
        auto newspan = span(_vector->data(), _vector->size());
        auto spr = ioruntime::SpanReader(newspan);
        spr.discard_first(other._spr->get_bytes_read());
        _spr = move(spr);
    }
    if (other._icf.has_value()) {
        _icf = ioruntime::RefIoCopyFuture(*_spr, *_temp);
    }
    return *this;
}

http::IncomingBody::IncomingBody(vector<uint8_t>&& buffer)
    : _vector(move(buffer))
{
    _size = _vector->size();
}

http::IncomingBody::IncomingBody()
    : _vector(vector<uint8_t>())
{
}

http::IncomingBody::~IncomingBody() = default;
