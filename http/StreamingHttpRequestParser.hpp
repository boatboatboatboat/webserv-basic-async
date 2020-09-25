//
// Created by boat on 13-09-20.
//

#ifndef WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
#define WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP

#include "../futures/PollResult.hpp"
#include "../ioruntime/IAsyncRead.hpp"
#include "../utils/utils.hpp"
#include "HttpMethod.hpp"
#include "HttpStatus.hpp"
#include "HttpVersion.hpp"
#include <map>
#include <string>
#include <vector>

using futures::PollResult;
using futures::StreamPollResult;
using ioruntime::IAsyncRead;
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
    auto method(HttpMethod&& method) -> StreamingHttpRequestBuilder&;
    auto uri(string&& uri) -> StreamingHttpRequestBuilder&;
    auto version(HttpVersion&& version) -> StreamingHttpRequestBuilder&;
    auto status(HttpStatus&& status) -> StreamingHttpRequestBuilder&;
    auto header(string&& header_name, string&& header_value) -> StreamingHttpRequestBuilder&;
    auto body(vector<uint8_t>&& body) -> StreamingHttpRequestBuilder&;
    auto build() && -> StreamingHttpRequest;

private:
    HttpMethod _method;
    string _uri;
    HttpVersion _version;
    HttpStatus _status;
    map<string, string> _headers;
    vector<uint8_t> _body;
};

class StreamingHttpRequest {
public:
    StreamingHttpRequest(
        HttpMethod method,
        string uri,
        HttpVersion version,
        HttpStatus status,
        map<string, string> headers,
        vector<uint8_t> body);
    auto get_method() -> HttpMethod const&;
    auto get_uri() -> string const&;
    auto get_version() -> HttpVersion const&;
    auto get_status() -> HttpStatus const&;
    auto get_headers() -> map<string, string> const&;
    auto get_body() -> vector<uint8_t> const&;

private:
    HttpMethod method;
    string uri;
    HttpVersion version;
    HttpStatus status;
    map<string, string> headers;
    vector<uint8_t> body;
};

inline auto is_method_valid(std::string_view method) -> bool
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

    return std::any_of(methods.begin(), methods.end(), [&](auto real) { return real.starts_with(method); });
}

class StreamingHttpRequestParser {
public:
    explicit StreamingHttpRequestParser(IAsyncRead& source, size_t buffer_limit, size_t body_limit)
        : buffer_limit(buffer_limit)
        , body_limit(body_limit)
        , source(source)
    {
        buffer.reserve(buffer_limit);
    }

    auto poll_parse(Waker&& waker) -> PollResult<StreamingHttpRequest>
    {
        auto poll_result = poll_character(std::move(waker));
        using Status = StreamPollResult<uint8_t>::Status;

        switch (poll_result.get_status()) {
        case Status::Ready: {
            auto result = poll_result.get();
            auto matched = false;
            switch (state) {
            case ReadMethod:
            case ReadUri: {
                if (result == ' ') {
                }
            } break;
            case ReadVersion:
            case HeaderLine: {

            } break;
            case Body: {

            } break;
            }
            if (result == '\n' && !buffer.empty() && buffer.back() == '\r') {
                // lijntje gepakt
                switch (state) {
                case RequestLine: {
                    std::string_view buf_view(reinterpret_cast<const char*>(buffer.data()), buffer.size());

                    auto method_end = buf_view.find(' ');
                    if (method_end == std::string::npos) {
                        throw std::runtime_error("parser error, no first space");
                    }
                    auto method = std::string_view(buf_view.data(), method_end);
                    if (!is_method_valid(method)) {
                        throw std::runtime_error("invalid method");
                    }
                    auto request_end = buf_view.rfind(' ');
                    if (request_end == std::string::npos) {
                        throw std::runtime_error("parser error, no second space");
                    }
                    auto request = std::string_view(buf_view.data() + method_end, request_end - method_end);
                    auto version = std::string_view(buf_view.data() + request_end, buf_view.length() - request_end);

                } break;
                case HeaderLine: {

                } break;
                case Body: {

                } break;
                }
            }
            buffer.push_back(poll_result.get());
            if (buffer.size() >= buffer_limit) {
                throw std::runtime_error("buffer exceeded limit");
            }
        } break;
        case Status::Pending: {
            return PollResult<StreamingHttpRequest>::pending();
        } break;
        case Status::Finished: {

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
        if (character_head == character_max)
            character_head = 0;
        return StreamPollResult<uint8_t>::ready(uint8_t(character_buffer[character_head - 1])); // LOL
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
    IAsyncRead& source;
    vector<uint8_t> buffer;
    StreamingHttpRequestBuilder builder;
};

/*class RequestLineParser {
public:
    auto try_parse(std::string_view& view) -> bool {
        switch (state) {
        case Method: {
            if (is_method_valid(view)) {}
        } break;
        }
        return false;
    };

private:
    static inline auto is_method_valid(std::string_view method) -> bool;
    static inline auto is_uri_valid(std::string_view method) -> bool;
    static inline auto is_version_valid(std::string_view method) -> bool;
    inline auto take_method() -> bool;
    inline auto take_uri() -> bool;
    inline auto take_space() -> bool;
    inline auto take_version() -> bool;
    inline auto take_crlf() -> bool;
    enum State {
        Method,
        Space1,
        Uri,
        Space2,
        Version,
        Clrf
    } state
        = Method;
    HttpMethod method;
    std::string uri;
    std::string version;
};

class HeaderLineParser {
public:
private:
};

class BodyParser {
public:
private:
};

class StreamingHttpRequestParser {
public:
    StreamingHttpRequestParser();

    virtual ~StreamingHttpRequestParser();
    bool parse(span<uint8_t> incoming);

    auto get_body() const -> span<uint8_t>;

    string_view get_header(string_view name) const;
    map<string, string> const& get_headers() const;
    string_view get_method() const;
    string_view get_url() const;
    string_view get_version() const;
    string_view get_status() const;
    string_view get_reason() const;
private:
    enum Tag {
        None,
        RequestLine,
        HeaderLine,
        Body
    } tag
        = None;
    union {
        RequestLineParser request_line_parser;
        HeaderLineParser header_line_parser;
        BodyParser body_parser;
    };
    enum UriType {
        Server,
        AbsoluteUri,
        AbsPath,
        Authority
    } uri_type;
    void next_parser();
    StreamingHttpRequestBuilder request;
};*/

}

#endif //WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
