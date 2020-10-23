//
// Created by boat on 10/23/20.
//

#ifndef WEBSERV_HTTP_INCOMINGRESPONSE_HPP
#define WEBSERV_HTTP_INCOMINGRESPONSE_HPP

#include "IncomingMessage.hpp"
#include "Status.hpp"
#include "Version.hpp"

namespace http {

class IncomingResponse final {
public:
    IncomingResponse() = delete;
    IncomingResponse(Version, Status, IncomingMessage&&);
    IncomingResponse(IncomingResponse&&) noexcept = default;
    auto operator=(IncomingResponse&&) noexcept -> IncomingResponse& = default;
    IncomingResponse(IncomingResponse const&) = delete;
    auto operator=(IncomingResponse const&) -> IncomingResponse& = delete;
    virtual ~IncomingResponse() = default;

    [[nodiscard]] auto get_version() const -> Version;
    [[nodiscard]] auto get_status() const -> Status;
    [[nodiscard]] auto get_message() -> IncomingMessage&;
    [[nodiscard]] auto get_message() const -> IncomingMessage const&;
private:
    Version _version;
    Status _status;
    IncomingMessage _message;
};

class IncomingResponseBuilder final {
public:
    IncomingResponseBuilder() = default;
    IncomingResponseBuilder(IncomingResponseBuilder&&) noexcept = default;
    auto operator=(IncomingResponseBuilder&&) noexcept -> IncomingResponseBuilder& = default;
    IncomingResponseBuilder(IncomingResponseBuilder const&) = delete;
    auto operator=(IncomingResponseBuilder const&) = delete;
    virtual ~IncomingResponseBuilder() = default;

    auto version(Version) -> IncomingResponseBuilder&;
    auto status(Status) -> IncomingResponseBuilder&;
    auto message(IncomingMessage&&) -> IncomingResponseBuilder&;

    auto build() && -> IncomingResponse;
private:
    optional<Version> _version;
    optional<Status> _status;
    optional<IncomingMessage> _message;
};

}

#endif //WEBSERV_HTTP_INCOMINGRESPONSE_HPP
