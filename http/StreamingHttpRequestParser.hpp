//
// Created by boat on 13-09-20.
//

#ifndef WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
#define WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP

#include "../futures/PollResult.hpp"
#include "../utils/utils.hpp"
#include "HttpMethod.hpp"
#include "HttpStatus.hpp"
#include "HttpVersion.hpp"
#include <map>
#include <string>
#include <vector>

using futures::PollResult;
using std::string_view;
using std::map;
using std::vector;
using utils::span;
using std::string;

namespace http {

class StreamingHttpRequestParser {
public:
    StreamingHttpRequestParser();
    virtual ~StreamingHttpRequestParser();

    bool parse(char* buffer, size_t size);

    auto get_body() const -> span<uint8_t>;
    string_view get_header(string_view name) const;
    map<string, string> const& get_headers() const;
    string_view get_method() const;
    string_view get_url() const;
    string_view get_version() const;
    string_view get_status() const;
    string_view get_reason() const;

private:
    class RequestLineParser {
    public:
        auto try_parse(std::string_view view) -> bool;
    private:
        static inline auto is_method_valid(std::string_view method) -> bool;
        static inline auto is_uri_valid(std::string_view method) -> bool;
        static inline auto is_version_valid(std::string_view method) -> bool;
        enum State {
            Method,
            Space1,
            Uri,
            Space2,
            Version,
            Clrf
        } state = Method;
        std::string method;
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
    HttpMethod method;
    string uri;
    HttpVersion version;
    HttpStatus status;
    string reason;
    string header;
    map<string, string> headers;
    vector<uint8_t> body;

};

}

#endif //WEBSERV_HTTP_STREAMINGHTTPREQUESTPARSER_HPP
