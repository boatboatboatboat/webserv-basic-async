//
// Created by boat on 13-09-20.
//

#ifndef WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
#define WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP

#include "../futures/PollResult.hpp"
#include "../ioruntime/IAsyncRead.hpp"
#include "../utils/utils.hpp"
#include "HttpMethod.hpp"
#include "HttpRequest.hpp"
#include "HttpStatus.hpp"
#include "HttpVersion.hpp"
#include "Uri.hpp"
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

inline auto get_method(const string_view possible_method) -> optional<http::HttpMethod>
{
    constexpr http::HttpMethod VALID_METHODS[8] = {
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
            return optional<http::HttpMethod>(method);
        }
    }
    return optional<http::HttpMethod>::none();
}

inline auto method_has_body(const string_view possible_method) -> bool
{
    return possible_method == http::method::POST
        || possible_method == http::method::PUT
        || possible_method == http::method::PATCH
        || possible_method == http::method::DELETE;
}

class HttpRequestParser : public IFuture<HttpRequest> {
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
    explicit HttpRequestParser(IAsyncRead& source, size_t buffer_limit, size_t body_limit);
    HttpRequestParser(HttpRequestParser&& other) noexcept;
    HttpRequestParser(HttpRequestParser&& other, IAsyncRead& new_stream) noexcept;
    auto poll(Waker&& waker) -> PollResult<HttpRequest> override;
    auto poll_character(Waker&& waker) -> StreamPollResult<uint8_t>;

private:
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
    } state
        = ReadMethod;
    uint8_t character_buffer[128] {};
    size_t character_head = 0;
    size_t character_max = 0;
    size_t buffer_limit;
    size_t body_limit;
    HttpMethod current_method;
    optional<uint64_t> content_length;
    size_t last_chunk_size;
    bool chunked;
    IAsyncRead& source;
    vector<uint8_t> buffer;
    optional<vector<uint8_t>> decoded_body;
    HttpRequestBuilder builder;
    bool moved = false;
};

}

#endif //WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
