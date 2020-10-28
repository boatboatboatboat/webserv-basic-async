//
// Created by boat on 10/25/20.
//

#ifndef WEBSERV_HTTP_REQUESTREADER_HPP
#define WEBSERV_HTTP_REQUESTREADER_HPP

#include "../ioruntime/IAsyncRead.hpp"
#include "MessageReader.hpp"
#include "OutgoingRequest.hpp"

namespace http {

namespace inner_readers {
    class RequestLineReader final : public ioruntime::IAsyncRead {
    public:
        RequestLineReader() = delete;
        RequestLineReader(RequestLineReader&&) = default;
        auto operator=(RequestLineReader&&) -> RequestLineReader& = delete;
        RequestLineReader(RequestLineReader const&) = delete;
        auto operator=(RequestLineReader const&) -> RequestLineReader& = delete;
        ~RequestLineReader() override = default;

        RequestLineReader(Method, Uri, Version);

        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        Method _method;
        Uri _uri;
        Version _version;
        string _strbuf;
        string_view _current;
    };
}

class RequestReader final : public ioruntime::IAsyncRead {
public:
    RequestReader() = delete;
    RequestReader(RequestReader&&) noexcept;
    auto operator=(RequestReader&&) noexcept -> RequestReader& = delete;
    RequestReader(RequestReader const&) = delete;
    auto operator=(RequestReader const&) noexcept -> RequestReader& = delete;
    ~RequestReader() override;

    RequestReader(RequestReader&& other, OutgoingRequest& request) noexcept;
    explicit RequestReader(OutgoingRequest&);

    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    OutgoingRequest& _request;
    enum State {
        RequestLineState,
        MessageState,
    } _state
        = RequestLineState;
    union {
        inner_readers::RequestLineReader _request_line;
        MessageReader _message;
    };
};

}
#endif //WEBSERV_HTTP_REQUESTREADER_HPP
