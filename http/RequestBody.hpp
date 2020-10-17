//
// Created by boat on 11-10-20.
//

#ifndef WEBSERV_HTTP_REQUESTBODY_HPP
#define WEBSERV_HTTP_REQUESTBODY_HPP

#include "../fs/File.hpp"
#include "../http/SpanReader.hpp"
#include "../ioruntime/IAsyncRead.hpp"
#include "../ioruntime/IAsyncWrite.hpp"
#include "../ioruntime/IoCopyFuture.hpp"
#include "../option/optional.hpp"
#include <vector>

namespace {
#ifdef DEBUG_REQUEST_BODY_ALWAYS_SWITCH
constexpr size_t REQUEST_BODY_SWITCH_TEMPORARY_SIZE = 1;
#else
constexpr size_t REQUEST_BODY_SWITCH_TEMPORARY_SIZE = 3000000;
#endif

using fs::File;
using ioruntime::IAsyncRead;
using ioruntime::IAsyncWrite;
using ioruntime::IoResult;
using option::optional;
using std::vector;
}

#ifdef DEBUG
#include "../ioruntime/IoEventHandler.hpp"
#endif

namespace http {

class RequestBody : public IAsyncRead, public IAsyncWrite {
public:
    RequestBody();
    explicit RequestBody(vector<uint8_t>&& buffer);
    RequestBody(RequestBody const&) = delete;
    RequestBody& operator=(RequestBody const&) = delete;
    RequestBody(RequestBody&& other) noexcept;
    RequestBody& operator=(RequestBody&& other) noexcept ;
    ~RequestBody() override;
    auto poll_read(span<uint8_t> buffer, Waker&& waker)
        -> PollResult<IoResult> override;
    auto poll_write(span<uint8_t> buffer, Waker&& waker)
        -> PollResult<IoResult> override;
    [[nodiscard]] auto size() const -> size_t;
#ifdef DEBUG
    [[nodiscard]] auto debug_body(ioruntime::IoEventHandler& ioe) -> vector<uint8_t>;
#endif

private:
    enum BodyType {
        VectorBody,
        TemporaryBody,
    } _body_type
        = VectorBody;
    size_t _size = 0;
    bool _converting = false;
    optional<std::vector<uint8_t>> _vector;
    optional<ioruntime::SpanReader> _spr;
    optional<fs::File> _temp;
    optional<ioruntime::RefIoCopyFuture> _icf;
    bool _should_rewind = false;
};

}

#endif //WEBSERV_HTTP_REQUESTBODY_HPP
