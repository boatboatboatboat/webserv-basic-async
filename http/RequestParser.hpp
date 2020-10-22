//
// Created by boat on 13-09-20.
//

#ifndef WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
#define WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP

#include "../futures/PollResult.hpp"
#include "../ioruntime/IAsyncRead.hpp"
#include "../utils/utils.hpp"
#include "Method.hpp"
#include "Request.hpp"
#include "Status.hpp"
#include "Uri.hpp"
#include "Version.hpp"
#include <map>
#include <string>
#include <vector>

using futures::PollResult;
using futures::StreamPollResult;
using ioruntime::IAsyncRead;
using option::optional;
using std::map;
using std::string;
using std::string_view;
using std::vector;
using utils::span;

namespace http {

inline auto get_method(const string_view possible_method) -> optional<http::Method>
{
    constexpr http::Method VALID_METHODS[8] = {
        http::method::CONNECT,
        http::method::DELETE,
        http::method::GET,
        http::method::HEAD,
        http::method::OPTIONS,
        http::method::PATCH,
        http::method::POST,
        http::method::PUT,
    };
    auto methods = span(VALID_METHODS, 8);

    for (auto& method : methods) {
        if (method == possible_method) {
            return optional<http::Method>(method);
        }
    }
    return optional<http::Method>::none();
}

inline auto method_has_body(const string_view possible_method) -> bool
{
    return possible_method == http::method::POST
        || possible_method == http::method::PUT
        || possible_method == http::method::PATCH
        || possible_method == http::method::DELETE;
}

class RequestParser : public IFuture<Request> {
public:
    class ParserError : public std::runtime_error {
    public:
        explicit ParserError(const char* w);
    };
    class RequestUriExceededBuffer : public ParserError {
    public:
        RequestUriExceededBuffer();
    };
    class BodyExceededLimit : public ParserError {
    public:
        BodyExceededLimit();
    };
    class GenericExceededBuffer : public ParserError {
    public:
        GenericExceededBuffer();
    };
    class InvalidMethod : public ParserError {
    public:
        InvalidMethod();
    };
    class InvalidVersion : public ParserError {
    public:
        InvalidVersion();
    };
    class MalformedRequest : public ParserError {
    public:
        MalformedRequest();
    };
    class UnexpectedEof : public ParserError {
    public:
        UnexpectedEof();
    };
    class UndeterminedLength : public ParserError {
    public:
        UndeterminedLength();
    };
    class BadTransferEncoding : public ParserError {
    public:
        BadTransferEncoding();
    };
    explicit RequestParser(IAsyncRead& source, size_t buffer_limit, size_t body_limit);
    RequestParser(RequestParser&& other) noexcept;
    RequestParser(RequestParser&& other, IAsyncRead& new_stream) noexcept;
    auto poll(Waker&& waker) -> PollResult<Request> override;
    auto poll_character(Waker&& waker) -> StreamPollResult<uint8_t>;

private:
    enum CallState {
        Pending,
        Running,
        Completed,
    };
    auto inner_poll(Waker&& waker) -> CallState;
    enum State {
        ReadMethod,
        ReadUri,
        ReadVersion,
        HeaderLine,

        // Regular body
        Body,

        // Chunked body
        ChunkedBodySize,
        ChunkedBodyData,

        // hack
        AppendToBody,
    } state
        = ReadMethod;
    uint8_t character_buffer[32192] {};
    size_t character_head = 0;
    size_t character_max = 0;
    bool character_buffer_exhausted = true;
    size_t buffer_limit;
    size_t body_limit;
    Method current_method;
    optional<uint64_t> content_length;
    size_t last_chunk_size;
    bool chunked;
    bool host_set = false;
    bool is_http_1_1 = false;
    IAsyncRead& source;
    vector<uint8_t> buffer;
    optional<ioruntime::SpanReader> span_reader;
    optional<ioruntime::RefIoCopyFuture> ricf;
    optional<IncomingBody> decoded_body;
    RequestBuilder builder;
    bool moved = false;
    bool shift_back = false;
};

}

#endif //WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
