//
// Created by boat on 8/20/20.
//

#ifndef WEBSERV_HTTPRESPONSE_HPP
#define WEBSERV_HTTPRESPONSE_HPP

#include "../cgi/Cgi.hpp"
#include "../ioruntime/IoCopyFuture.hpp"
#include "../net/Socket.hpp"
#include "DefaultPageReader.hpp"
#include "Header.hpp"
#include "IncomingRequest.hpp"
#include "MessageReader.hpp"
#include "OutgoingBody.hpp"
#include "RfcConstants.hpp"
#include "Status.hpp"
#include "Version.hpp"
#include <map>
#include <string>

using cgi::Cgi;

namespace http {

class OutgoingResponse;

class OutgoingResponseBuilder final {
public:
    OutgoingResponseBuilder();
    auto version(Version version) -> OutgoingResponseBuilder&;
    auto status(Status status) -> OutgoingResponseBuilder&;
    auto header(HeaderName name, const HeaderValue& value) -> OutgoingResponseBuilder&;
    auto headers(Headers&& headers) -> OutgoingResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body) -> OutgoingResponseBuilder&;
    auto body(BoxPtr<ioruntime::IAsyncRead>&& body, size_t content_length) -> OutgoingResponseBuilder&;
    auto cgi(Cgi&& proc) -> OutgoingResponseBuilder&;
    [[nodiscard]] auto build() && -> OutgoingResponse;

private:
    Version _version = version::v1_1;
    optional<Status> _status;
    OutgoingMessageBuilder _message;
};

class OutgoingResponse final {
public:
    // ctors/dtors
    OutgoingResponse() = delete;
    OutgoingResponse(Version version, Status status, OutgoingMessage&& message);
    ~OutgoingResponse() = default;
    OutgoingResponse(OutgoingResponse&&) noexcept = default;
    auto operator=(OutgoingResponse&&) noexcept -> OutgoingResponse& = default;
    OutgoingResponse(OutgoingResponse const&) = delete;
    auto operator=(OutgoingResponse const&) -> OutgoingResponse& = delete;

    // getter methods
    [[nodiscard]] auto get_headers() const -> Headers const&;
    [[nodiscard]] auto get_header(HeaderName const& needle_name) const -> optional<HeaderValue>;
    [[nodiscard]] auto get_version() const -> Version const&;
    [[nodiscard]] auto get_status() const -> Status const&;
    [[nodiscard]] auto get_body() const -> optional<OutgoingBody> const&;
    [[nodiscard]] auto get_body() -> optional<OutgoingBody>&;
    [[nodiscard]] auto get_message() -> OutgoingMessage&;
    [[nodiscard]] auto get_message() const -> OutgoingMessage const&;

    void drop_body();

private:
    Version _version;
    Status _status;
    OutgoingMessage _message;
};

class ResponseReader final : public IAsyncRead {
public:
    ResponseReader() = delete;
    ResponseReader(ResponseReader&&) noexcept;
    auto operator=(ResponseReader&&) noexcept -> ResponseReader& = delete;
    ResponseReader(ResponseReader const&) = delete;
    auto operator=(ResponseReader const&) -> ResponseReader& = delete;
    ~ResponseReader() override;

    explicit ResponseReader(OutgoingResponse& response);
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    // inner readers

    // reads the status line
    class StatusLineReader final : public IAsyncRead {
    public:
        StatusLineReader() = delete;
        explicit StatusLineReader(Version const& version, Status const& status);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        enum State {
            VersionState,
            Space1,
            Code,
            Space2,
            Message,
            Clrf,
        } _state
            = VersionState;
        char _status_code_buf[3];
        string_view _current;
        Version const& _version;
        Status const& _status;
    };

    // reads the body from cgi
    class CgiReader final : public IAsyncRead {
    public:
        CgiReader() = delete;
        explicit CgiReader(Cgi& cgi);
        auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

    private:
        enum State {
            WriteRequestBody,
            ReadCgi,
        } _state
            = WriteRequestBody;
        bool writer_finished = false;
        Cgi& _cgi;
    };

    // Members
    OutgoingResponse& _response;
    enum State {
        CheckCgiState,
        StatusLineReaderState,
        MessageState,
        CgiState,
    } _state;
    union {
        StatusLineReader _status_line;
        MessageReader _message;
        CgiReader _cgi_reader;
    };
};

}

#endif //WEBSERV_HTTPRESPONSE_HPP
