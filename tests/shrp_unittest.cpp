//
// Created by boat on 9/27/20.
//

#include "../func/Functor.hpp"
#include "../http/StreamingHttpRequestParser.hpp"
#include "../http/StringBody.hpp"
#include "../ioruntime/RuntimeBuilder.hpp"

#include "gtest/gtest.h"

using namespace http;
using namespace std;
using namespace ioruntime;

namespace {

TEST(ShrpTests, shrp_basic_request)
{
    const string request = "GET / HTTP/1.1\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_EQ(val.get_version().version_string, HTTP_VERSION_1_1.version_string);
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_request_path)
{
    const string request = "GET /example.html HTTP/1.1\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/example.html");
            EXPECT_EQ(val.get_version().version_string, HTTP_VERSION_1_1.version_string);
            break;
        }
    }
}

TEST(ShrpTests, shrp_request_asterisk)
{
    const string request = "OPTIONS * HTTP/1.1\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::OPTIONS);
            EXPECT_EQ(val.get_uri().get_target_form(), Uri::AsteriskForm);
            EXPECT_EQ(val.get_version().version_string, HTTP_VERSION_1_1.version_string);
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_authority)
{
    const string request = "CONNECT www.example.com:80 HTTP/1.1\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::CONNECT);
            EXPECT_EQ(val.get_uri().get_authority()->get_host(), "www.example.com");
            EXPECT_EQ(val.get_uri().get_authority()->get_port().value(), 80);
            EXPECT_EQ(val.get_version().version_string, HTTP_VERSION_1_1.version_string);
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_body)
{
    const string request = "POST /form?inline=test HTTP/1.1\r\nContent-Length: 13\r\n\r\nHello, world!";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, HTTP_VERSION_1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_body().data()), val.get_body().size()),
                "Hello, world!");
            break;
        }
    }
}

TEST(ShrpTests, shrp_body_limit)
{
    const string request = "POST /form?inline=test HTTP/1.1\r\nContent-Length: 13\r\n\r\nHello, world!";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 10;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
        }
    },
        StreamingHttpRequestParser::BodyExceededLimit);
}

TEST(ShrpTests, shrp_buffer_limit_uri)
{
    const string request = "POST /form?inline=test HTTP/1.1\r\n\r\nHello, world!";
    const size_t buffer_limit = 10;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
        }
    },
        StreamingHttpRequestParser::RequestUriExceededBuffer);
}

TEST(ShrpTests, shrp_buffer_limit_generic)
{
    const string request = "ABCDEFGHIJKLMOPQRS /form?inline=test HTTP/1.1\r\n\r\nHello, world!";
    const size_t buffer_limit = 10;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
        }
    },
        StreamingHttpRequestParser::GenericExceededBuffer);
}

TEST(ShrpTests, shrp_get_header)
{
    const string request = "GET / HTTP/1.1\r\nX-Useless-Header: false\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query());
            EXPECT_EQ(val.get_version().version_string, HTTP_VERSION_1_1.version_string);
            EXPECT_EQ(val.get_headers().size(), 1);
            EXPECT_EQ(val.get_headers().find("X-Useless-Header")->second, "false");
            break;
        }
    }
}

TEST(ShrpTests, shrp_get_header_n2)
{
    const string request = "GET / HTTP/1.1\r\nX-Useless-Header: false\r\nX-More-Useless-Header: funny\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query());
            EXPECT_EQ(val.get_version().version_string, HTTP_VERSION_1_1.version_string);
            EXPECT_EQ(val.get_headers().size(), 2);
            EXPECT_EQ(val.get_headers().find("X-Useless-Header")->second, "false");
            EXPECT_EQ(val.get_headers().find("X-More-Useless-Header")->second, "funny");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_buffer_limit)
{
    const string request = "GET / HTTP/1.1\r\nX-Useless-Header: false\r\nX-More-Useless-Header: funny\r\n\r\n";
    const size_t buffer_limit = 10;
    const size_t body_limit = 8192;

    StringBody sbody(request);

    StreamingHttpRequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
        }
    },
        StreamingHttpRequestParser::GenericExceededBuffer);
}
}
