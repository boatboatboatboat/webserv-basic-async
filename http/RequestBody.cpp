//
// Created by boat on 11-10-20.
//

#include "RequestBody.hpp"

using std::move;

auto http::RequestBody::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    switch (_body_type) {
        case VectorBody: {
            if (!_spr.has_value()) {
                _spr = ioruntime::SpanReader(span(_vector->data(), _size));
            }
            return _spr->poll_read(buffer, move(waker));
        } break;
        case TemporaryBody: {
            if (_should_rewind) {
                _should_rewind = false;
                lseek(_temp->get_descriptor(), 0, SEEK_SET);
            }
            return _temp->poll_read(buffer, move(waker));
        } break;
    }
}

auto http::RequestBody::poll_write(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
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

auto http::RequestBody::size() const -> size_t
{
    return _size;
}

http::RequestBody::RequestBody(http::RequestBody&& other) noexcept
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

http::RequestBody& http::RequestBody::operator=(http::RequestBody&& other) noexcept
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

http::RequestBody::RequestBody(vector<uint8_t>&& buffer)
    : _vector(move(buffer))
{
    _size = _vector->size();
}

#ifdef DEBUG

#include "../ioruntime/IoEventHandler.hpp"
using ioruntime::IoEventHandler;

// this function is only used in unit tests
auto http::RequestBody::debug_body(IoEventHandler& ioe) -> vector<uint8_t>
{
    vector<uint8_t> a;
    uint8_t f[8192];
    while (true) {
        ioe.reactor_step();
        auto r = poll_read(span(f, sizeof(f)), Waker::dead());

        if (r.is_ready()) {
            auto s = r.get();
            if (s.is_error()) {
                throw std::runtime_error("data pr err");
            } else if (s.is_eof()) {
                return a;
            } else {
                span found(f, s.get_bytes_read());
                a.insert(a.end(), found.begin(), found.end());
            }
        }
    }
}

http::RequestBody::RequestBody()
    : _vector(vector<uint8_t>())
{
}

http::RequestBody::~RequestBody() = default;

#endif
