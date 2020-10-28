//
// Created by boat on 9/27/20.
//

#include "../func/Functor.hpp"
#include "../http/RequestParser.hpp"
#include "../http/StringReader.hpp"
#include "../ioruntime/RuntimeBuilder.hpp"

#include "../ioruntime/GlobalTimeoutEventHandler.hpp"
#include "gtest/gtest.h"

using namespace http;
using namespace std;
using namespace ioruntime;

namespace {

TEST(ShrpTests, shrp_basic_request)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_request_path)
{
    const string request = "GET /example.html HTTP/1.1\r\nHost: test\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/example.html");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            break;
        }
    }
}

TEST(ShrpTests, shrp_request_asterisk)
{
    const string request = "OPTIONS * HTTP/1.1\r\nHost: test\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::OPTIONS);
            EXPECT_EQ(val.get_uri().get_target_form(), Uri::AsteriskForm);
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_authority)
{
    const string request = "CONNECT www.example.com:80 HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::CONNECT);
            EXPECT_EQ(val.get_uri().get_authority()->get_host(), "www.example.com");
            EXPECT_EQ(val.get_uri().get_authority()->get_port().value(), 80);
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_body)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nContent-Length: 13\r\n\r\nHello, world!";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        ioe.reactor_step();
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "Hello, world!");
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_without_body)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nContent-Length: 0\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        ioe.reactor_step();
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "");
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_chunked_body)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nTRANSFER-encoding:chunked\r\n\r\n5\r\nHello\r\n2\r\n, \r\n6\r\nworld!\r\n0\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        ioe.reactor_step();
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "Hello, world!");
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_chunked_body_sc_fakecap1)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nTransfer-Encoding: chunked\r\n\r\nb\r\nabcdefghijk\r\n0\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true, 1);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        ioe.reactor_step();
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "abcdefghijk");
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_chunked_body_sc_fakecap2)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nTransfer-Encoding: chunked\r\n\r\nb\r\nabcdefghijk\r\n0\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true, 2);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        ioe.reactor_step();
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "abcdefghijk");
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_chunked_body_sc_fakecap3)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nTransfer-Encoding: chunked\r\n\r\nb\r\nabcdefghijk\r\n0\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true, 3);
    auto sbody = ioruntime::CharacterStream(srbody);
    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        ioe.reactor_step();
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "abcdefghijk");
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_chunked_body_shk)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nTransfer-Encoding: chunked\r\n\r\nb\r\nabcdefghijk\r\n2\r\nlm\r\n6\r\nnopqrs\r\n0\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        ioe.reactor_step();
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "abcdefghijklmnopqrs");
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_chunked_body_ddk)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nTransfer-Encoding: chunked\r\n\r\n11\r\n0123456789ABCDEF1\r\n2\r\nlm\r\n6\r\nnopqrs\r\n0\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        ioe.reactor_step();
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "0123456789ABCDEF1lmnopqrs");
            break;
        }
    }
}

TEST(ShrpTests, shrp_basic_post_with_chunked_body_ones)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\n"
                           "Host: test\r\n"
                           "TRANSFER-encoding:chunked\r\n"
                           "\r\n"
                           "1\r\na\r\n"
                           "1\r\nb\r\n"
                           "2\r\ncd\r\n"
                           "1\r\ne\r\n"
                           "1\r\nf\r\n"
                           "2\r\ngh\r\n"
                           "1\r\ni\r\n"
                           "1\r\nj\r\n"
                           "2\r\nkl\r\n"
                           "1\r\nm\r\n"
                           "0\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true, 6);
    auto sbody = ioruntime::CharacterStream(srbody);
    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        ioe.reactor_step();
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::POST);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/form");
            EXPECT_EQ(val.get_uri().get_pqf().value().get_query().value(), "inline=test");
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(
                string_view(reinterpret_cast<const char*>(val.get_message().get_body()->debug_body(ioe).data()), val.get_message().get_body()->size()),
                "abcdefghijklm");
            break;
        }
    }
}

TEST(ShrpTests, shrp_body_limit)
{
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\nContent-Length: 13\r\n\r\nHello, world!";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 10;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            ioe.reactor_step();
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::BodyExceededLimit);
}

TEST(ShrpTests, shrp_buffer_limit_uri)
{
    const string request = "POST /form?inline=test HTTP/1.1\r\nHost: test\r\n\r\n";
    const size_t buffer_limit = 10;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        RequestParser::UriExceededBuffer);
}

TEST(ShrpTests, shrp_buffer_limit_generic)
{
    const string request = "ABCDEFGHIJKLMOPQRS /form?inline=test HTTP/1.1\r\nHost: test\r\n\r\nHello, world!";
    const size_t buffer_limit = 10;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::GenericExceededBuffer);
}

TEST(ShrpTests, shrp_get_header)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nX-Useless-Header: false\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("X-Useless-Header").value(), "false");
            break;
        }
    }
}

TEST(ShrpTests, shrp_get_header_n2)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nX-Useless-Header: false\r\nX-More-Useless-Header: request_perl_tests\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 3);
            EXPECT_EQ(val.get_message().get_header("X-Useless-Header").value(), "false");
            EXPECT_EQ(val.get_message().get_header("X-More-Useless-Header").value(), "request_perl_tests");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_buffer_limit)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nX-Useless-Header: false\r\nX-More-Useless-Header: request_perl_tests\r\n\r\n";
    const size_t buffer_limit = 10;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::GenericExceededBuffer);
}

TEST(ShrpTests, shrp_empty_header_name_error)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\n: a\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_empty_header_name_untrimmed_error_s1)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\n : a\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_empty_header_name_untrimmed_error_s2)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\n  : a\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_empty_header_value)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nX-Useless-Header:\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("X-Useless-Header").value(), "");
            break;
        }
    }
}

TEST(ShrpTests, shrp_empty_header_value_untrimmed)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nX-Useless-Header:   \r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("X-Useless-Header").value(), "");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_user_agent_valid)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nUser-Agent: webserv_utests\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("User-Agent").value(), "webserv_utests");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_user_agent_valid_productver)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nUser-Agent: webserv_utests/42.42ft\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("User-Agent").value(), "webserv_utests/42.42ft");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_user_agent_invalid_empty)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nUser-Agent:\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_user_agent_invalid_no_product)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nUser-Agent: /hello\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_user_agent_invalid_no_product_version)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nUser-Agent: hello/\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_user_agent_invalid_multi_no_product)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nUser-Agent: te/st /hello te/st\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_user_agent_invalid_multi_no_product_version)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nUser-Agent: te/st hello/ te/st\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW({
        while (true) {
            auto res = parser.poll(Waker::dead());
            ASSERT_FALSE(res.is_ready());
        }
    },
        MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_accept_charset_valid)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: webserv_utests\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("Accept-Charset").value(), "webserv_utests");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_accept_charset_valid_odd)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: someset , SOMESET;q=0 ,some-set ; q=1.123\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("Accept-Charset").value(), "someset , SOMESET;q=0 ,some-set ; q=1.123");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_accept_charset_valid_common)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: utf-8;q=1, iso-8859-1;q=0.8\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("Accept-Charset").value(), "utf-8;q=1, iso-8859-1;q=0.8");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_accept_charset_valid_wildcard)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: utf-8;q=1, iso-8859-1;q=0.8, *\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("Accept-Charset").value(), "utf-8;q=1, iso-8859-1;q=0.8, *");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_accept_charset_many_wildcard)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: *, * , *;q=0.5\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("Accept-Charset").value(), "*, * , *;q=0.5");
            break;
        }
    }
}

TEST(ShrpTests, shrp_header_accept_charset_invalid_empty)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset:\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW(
        while (true) {
            auto res = parser.poll(Waker::dead());
            if (res.is_ready()) {
                throw std::runtime_error("success");
                break;
            }
        },
        http::MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_accept_charset_invalid_start_comma)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: ,hello\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW(
        while (true) {
            auto res = parser.poll(Waker::dead());
            if (res.is_ready()) {
                throw std::runtime_error("success");
                break;
            }
        },
        http::MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_accept_charset_invalid_inner_spaced)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: hello,  hello\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW(
        while (true) {
            auto res = parser.poll(Waker::dead());
            if (res.is_ready()) {
                throw std::runtime_error("success");
                break;
            }
        },
        http::MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_accept_charset_invalid_comma_empty)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: hello,\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW(
        while (true) {
            auto res = parser.poll(Waker::dead());
            if (res.is_ready()) {
                throw std::runtime_error("success");
                break;
            }
        },
        http::MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_accept_charset_invalid_comma_empty_space)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Charset: hello, \r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW(
        while (true) {
            auto res = parser.poll(Waker::dead());
            if (res.is_ready()) {
                throw std::runtime_error("success");
                break;
            }
        },
        http::MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_accept_language_invalid_wildcard)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Language: t3-st\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    EXPECT_THROW(
        while (true) {
            auto res = parser.poll(Waker::dead());
            if (res.is_ready()) {
                throw std::runtime_error("success");
                break;
            }
        },
        http::MessageParser::MalformedMessage);
}

TEST(ShrpTests, shrp_header_accept_language_valid)
{
    const string request = "GET / HTTP/1.1\r\nHost: test\r\nAccept-Language: nl;q=1, en-GB;q=0.9, en-US, *\r\n\r\n";
    const size_t buffer_limit = 8192;
    const size_t body_limit = 8192;

    StringReader srbody(request, true);
    auto sbody = ioruntime::CharacterStream(srbody);

    RequestParser parser(sbody, buffer_limit, body_limit);

    while (true) {
        auto res = parser.poll(Waker::dead());
        if (res.is_ready()) {
            auto val = std::move(res.get());
            EXPECT_EQ(val.get_method(), method::GET);
            EXPECT_EQ(val.get_uri().get_pqf().value().get_path().value(), "/");
            EXPECT_FALSE(val.get_uri().get_pqf().value().get_query().has_value());
            EXPECT_EQ(val.get_version().version_string, http::version::v1_1.version_string);
            EXPECT_EQ(val.get_message().get_headers().size(), 2);
            EXPECT_EQ(val.get_message().get_header("Accept-Language").value(), "nl;q=1, en-GB;q=0.9, en-US, *");
            break;
        }
    }
}

}
