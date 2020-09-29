//
// Created by boat on 13-09-20.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#ifndef WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
#define WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP

#include "../futures/PollResult.hpp"
#include "../ioruntime/IAsyncRead.hpp"
#include "../utils/utils.hpp"
#include "HttpMethod.hpp"
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

class StreamingHttpRequest;

class StreamingHttpRequestBuilder {
public:
    StreamingHttpRequestBuilder() = default;
    auto method(HttpMethod method) -> StreamingHttpRequestBuilder&;
    auto uri(Uri&& uri) -> StreamingHttpRequestBuilder&;
    auto version(HttpVersion version) -> StreamingHttpRequestBuilder&;
    auto header(string&& header_name, string&& header_value) -> StreamingHttpRequestBuilder&;
    auto body(vector<uint8_t>&& body) -> StreamingHttpRequestBuilder&;
    auto build() && -> StreamingHttpRequest;

private:
    HttpMethod _method;
    optional<Uri> _uri;
    HttpVersion _version;
    map<string, string> _headers;
    vector<uint8_t> _body;
};

class StreamingHttpRequest {
public:
    StreamingHttpRequest(
        HttpMethod method,
        Uri uri,
        HttpVersion version,
        map<string, string> headers,
        vector<uint8_t> body);
    [[nodiscard]] auto get_method() const -> HttpMethod const&;
    [[nodiscard]] auto get_uri() const -> Uri const&;
    [[nodiscard]] auto get_version() const -> HttpVersion const&;
    [[nodiscard]] auto get_headers() const -> map<string, string> const&;
    [[nodiscard]] auto get_header(string_view name) const -> optional<string_view>;
    [[nodiscard]] auto get_body() const -> vector<uint8_t> const&;

private:
    HttpMethod _method;
    Uri _uri;
    HttpVersion _version;
    map<string, string> _headers;
    vector<uint8_t> _body;
};

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

class StreamingHttpRequestParser : public IFuture<StreamingHttpRequest> {
public:
    class ParserError : public std::runtime_error {
    public:
        explicit ParserError(const char* w)
            : std::runtime_error(w)
        {
        }
    };
    class RequestUriExceededBuffer : public ParserError {
    public:
        RequestUriExceededBuffer()
            : ParserError("Request URI exceeded buffer limit")
        {
        }
    };
    class BodyExceededLimit : public ParserError {
    public:
        BodyExceededLimit()
            : ParserError("Body exceeded limit")
        {
        }
    };
    class GenericExceededBuffer : public ParserError {
    public:
        GenericExceededBuffer()
            : ParserError("Exceeded buffer limit")
        {
        }
    };
    class InvalidMethod : public ParserError {
    public:
        InvalidMethod()
            : ParserError("Invalid HTTP method")
        {
        }
    };
    class InvalidVersion : public ParserError {
    public:
        InvalidVersion()
            : ParserError("Invalid HTTP version")
        {
        }
    };
    class MalformedHeader : public ParserError {
    public:
        MalformedHeader()
            : ParserError("Malformed HTTP header")
        {
        }
    };
    class UnexpectedEof : public ParserError {
    public:
        UnexpectedEof()
            : ParserError("Unexpected end of request")
        {
        }
    };
    class UndeterminedLength : public ParserError {
    public:
        UndeterminedLength()
            : ParserError("Undetermined length of body")
        {
        }
    };
    explicit StreamingHttpRequestParser(IAsyncRead& source, size_t buffer_limit, size_t body_limit)
        : buffer_limit(buffer_limit)
        , body_limit(body_limit)
        , source(source)
    {
    }

    StreamingHttpRequestParser(StreamingHttpRequestParser&& other) noexcept
        : character_head(other.character_head)
        , character_max(other.character_max)
        , buffer_limit(other.buffer_limit)
        , body_limit(other.body_limit)
        , source(other.source)
        , buffer(std::move(other.buffer))
        , builder(std::move(other.builder))
    {
        utils::memcpy(character_buffer, other.character_buffer);
        other.moved = true;
    }

    StreamingHttpRequestParser(StreamingHttpRequestParser&& other, IAsyncRead& new_stream) noexcept
        : character_head(other.character_head)
        , character_max(other.character_max)
        , buffer_limit(other.buffer_limit)
        , body_limit(other.body_limit)
        , source(new_stream)
        , buffer(std::move(other.buffer))
        , builder(std::move(other.builder))
    {
        utils::memcpy(character_buffer, other.character_buffer);
        other.moved = true;
    }

    auto poll(Waker&& waker) -> PollResult<StreamingHttpRequest> override
    {
        if (moved)
            return PollResult<StreamingHttpRequest>::pending();
        auto poll_result = poll_character(Waker(waker));
        using Status = StreamPollResult<uint8_t>::Status;

        switch (poll_result.get_status()) {
        case Status::Ready: {
            auto result = poll_result.get();
            auto matched = false;
            switch (state) {
            case ReadMethod:
            case ReadUri: {
                matched = result == ' ';
            } break;
            case ReadVersion:
            case HeaderLine: {
                matched = result == '\n' && !buffer.empty() && buffer.back() == '\r';
                if (matched) {
                    buffer.pop_back();
                }
            } break;
            case Body: {
                matched = true;
            }
                break;
            }
            if (state == Body && buffer.size() == body_limit) {
                throw BodyExceededLimit();
            } else if (buffer.size() == buffer_limit) {
                if (state == ReadUri) {
                    throw RequestUriExceededBuffer();
                } else {
                    throw GenericExceededBuffer();
                }
            }
            if (!matched || state == Body) {
                buffer.push_back(result);
            }
            if (matched) {
                string_view str_in_buf(reinterpret_cast<const char*>(buffer.data()), buffer.size());
                switch (state) {
                case ReadMethod: {
                    auto method = get_method(str_in_buf);
                    if (method.has_value()) {
                        current_method = *method;
                        builder.method(string_view(*method));
                    } else {
                        throw InvalidMethod();
                    }
                    buffer.clear();
                    state = ReadUri;
                } break;
                case ReadUri: {
                    auto uri = Uri(str_in_buf);
                    builder.uri(std::move(uri));
                    buffer.clear();
                    state = ReadVersion;
                } break;
                case ReadVersion: {
                    if (str_in_buf == HTTP_VERSION_1_0.version_string) {
                        builder.version(HTTP_VERSION_1_0);
                    } else if (str_in_buf == HTTP_VERSION_1_1.version_string) {
                        builder.version(HTTP_VERSION_1_1);
                    } else {
                        throw InvalidVersion();
                    }
                    buffer.clear();
                    state = HeaderLine;
                } break;
                case HeaderLine: {
                    if (str_in_buf.empty()) {
                        if (!method_has_body(current_method)) {
                            return PollResult<StreamingHttpRequest>::ready(std::move(builder).build());
                        }
                        state = Body;
                    } else {
                        auto separator = str_in_buf.find(':');

                        if (separator == string_view::npos) {
                            throw MalformedHeader();
                        }

                        auto field_name = str_in_buf.substr(0, separator);
                        auto field_value = str_in_buf.substr(separator + 1);

                        while (field_value.front() == ' ' || field_value.front() == '\t') {
                            field_value.remove_prefix(1);
                        }
                        while (field_value.back() == ' ' || field_value.back() == '\t') {
                            field_value.remove_suffix(1);
                        }

                        if (utils::str_eq_case_insensitive(field_name, "Content-Length")) {
                            auto length = utils::string_to_uint64(field_value);

                            if (length.has_value()) {
                                content_length = *length;
                            } else {
                                throw MalformedHeader();
                            }
                        }

                        builder.header(string(field_name), string(field_value));
                    }
                    buffer.clear();
                } break;
                case Body: {
                    if (content_length.has_value()) {
                        // CL
                        if (buffer.size() == *content_length) {
                            builder.body(std::move(buffer));
                            return PollResult<StreamingHttpRequest>::ready(std::move(builder).build());
                        }
                    } else {
                        // TCE
                        throw UndeterminedLength();
                    }
                } break;
                }
            }
            return poll(std::move(waker));
        } break;
        case Status::Pending: {
            return PollResult<StreamingHttpRequest>::pending();
        } break;
        case Status::Finished: {
            // All EOFs are unexpected, we want either TCE or CL
            throw UnexpectedEof();
        } break;
        }
    };

    auto poll_character(Waker&& waker) -> StreamPollResult<uint8_t>
    {
        if (character_head == 0) {
            auto poll_result = source.poll_read(reinterpret_cast<char*>(&character_buffer), sizeof(character_buffer), std::move(waker));

            if (poll_result.is_ready()) {
                auto result = poll_result.get();
                if (result < 0) {
                    throw std::runtime_error("StreamingHttpRequestParser: poll_character: read returned negative");
                }
                character_max = result;
            } else {
                return StreamPollResult<uint8_t>::pending();
            }
        }
        character_head += 1;
        if (character_max == 0)
            return StreamPollResult<uint8_t>::finished();
        uint8_t c = character_buffer[character_head - 1];
        if (character_head == character_max)
            character_head = 0;
        return StreamPollResult<uint8_t>::ready(uint8_t(c)); // LOL
    }

private:
    enum State {
        ReadMethod,
        ReadUri,
        ReadVersion,
        HeaderLine,
        Body
    } state
        = ReadMethod;
    uint8_t character_buffer[128] {};
    size_t character_head = 0;
    size_t character_max = 0;
    size_t buffer_limit;
    size_t body_limit;
    HttpMethod current_method;
    optional<uint64_t> content_length;
    IAsyncRead& source;
    vector<uint8_t> buffer;
    StreamingHttpRequestBuilder builder;
    bool moved = false;
};

}

#endif //WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP

#pragma clang diagnostic pop