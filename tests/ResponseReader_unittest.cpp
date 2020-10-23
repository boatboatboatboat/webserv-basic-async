//
// Created by boat on 10/5/20.
//

#include "../http/OutgoingResponse.hpp"
#include "../http/StringReader.hpp"
#include "../ioruntime/GlobalIoEventHandler.hpp"
#include "../ioruntime/GlobalTimeoutEventHandler.hpp"
#include "../ioruntime/IoEventHandler.hpp"
#include "../ioruntime/TimeoutEventHandler.hpp"
#include "gtest/gtest.h"

using namespace http;
using std::move;
using std::string;

static auto rr_read_to_string(OutgoingResponse&& response) -> string
{
    auto reader = ResponseReader(response);
    uint8_t ibuffer[8192];
    string full;
    auto buf = span(ibuffer, sizeof(ibuffer));

    while (true) {
        auto poll_result = reader.poll_read(buf, Waker::dead());
        if (poll_result.is_ready()) {
            auto result = poll_result.get();
            EXPECT_FALSE(result.is_error());
            if (result.is_success()) {
                full += string_view(reinterpret_cast<const char*>(buf.data()), result.get_bytes_read());
            } else if (result.is_eof()) {
                break;
            }
        }
    }
    return full;
}

auto rr_read_to_string_ioenabled_inner(ioruntime::IoEventHandler& ioe, OutgoingResponse&& response) -> string
{
    auto reader = ResponseReader(response);
    uint8_t ibuffer[8192];
    string full;

    auto buf = span(ibuffer, sizeof(ibuffer));
    while (true) {
        ioe.reactor_step();
        auto poll_result = reader.poll_read(buf, Waker::dead());
        if (poll_result.is_ready()) {
            auto result = poll_result.get();
            EXPECT_FALSE(result.is_error());
            if (result.is_success()) {
                full += string_view(reinterpret_cast<const char*>(buf.data()), result.get_bytes_read());
            } else if (result.is_eof()) {
                break;
            }
        }
    }
    return full;
}

auto rr_read_to_string_ioenabled(ioruntime::IoEventHandler& ioe, OutgoingResponse&& response) -> string
{
    ioruntime::GlobalIoEventHandler::set(&ioe);

    auto x = rr_read_to_string_ioenabled_inner(ioe, move(response));
    return x;
}

namespace {
TEST(ResponseReaderTests, rr_status_ok)
{
    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .build();

    string result = rr_read_to_string(move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\n\r\n");
}

TEST(ResponseReaderTests, rr_status_ise)
{
    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::INTERNAL_SERVER_ERROR)
                        .build();

    string result = rr_read_to_string(move(response));
    EXPECT_EQ(result, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
}

TEST(ResponseReaderTests, rr_header_1)
{
    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .header(header::CONNECTION, "close")
                        .build();

    string result = rr_read_to_string(move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n");
}

TEST(ResponseReaderTests, rr_header_2)
{
    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .header("X-Useless-Header", "request_perl_tests")
                        .header("X-Useless-Header2", "funny2")
                        .build();

    string result = rr_read_to_string(move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\nX-Useless-Header: request_perl_tests\r\nX-Useless-Header2: funny2\r\n\r\n");
}

TEST(ResponseReaderTests, rr_header_3)
{
    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .header("X-A", "aa")
                        .header("X-B", "bb")
                        .header("X-C", "cc")
                        .build();

    string result = rr_read_to_string(move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\nX-A: aa\r\nX-B: bb\r\nX-C: cc\r\n\r\n");
}

TEST(ResponseReaderTests, rr_body_1)
{
    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .body(BoxPtr<StringReader>::make("hello"), 5)
                        .build();

    string result = rr_read_to_string(move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello");
}

TEST(ResponseReaderTests, rr_body_2)
{
    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .header("X-A", "aa")
                        .body(BoxPtr<StringReader>::make("Hello, world!"), 13)
                        .build();

    string result = rr_read_to_string(move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\nX-A: aa\r\nContent-Length: 13\r\n\r\nHello, world!");
}

TEST(ResponseReaderTests, rr_chunked_body_1)
{
    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .body(BoxPtr<StringReader>::make("Hello, world!"))
                        .build();

    string result = rr_read_to_string(move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\nd\r\nHello, world!\r\n0\r\n\r\n");
}

TEST(ResponseReaderTests, rr_cgi_body_basic)
{
    // fake ioev
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    // fake request data
    auto reqbuilder = IncomingRequestBuilder();
    reqbuilder.uri(Uri("example.com")).method("GET").version(version::v1_1);
    auto request = move(reqbuilder).build();
    sockaddr_in funny { .sin_port = 0, .sin_addr = { 0 } };

    cgi::CgiServerForwardInfo csfi {
        .sockaddr = net::SocketAddr(funny),
        .server_name = "aaaa",
        .server_port = 1234,
    };

    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .cgi(Cgi("./tests/cgi_test/hello_world.lua", move(request), csfi))
                        .build();

    string result = rr_read_to_string_ioenabled(ioe, move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nhello world");
}

TEST(ResponseReaderTests, rr_cgi_fail)
{
    // fake ioev
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    // fake request data
    auto reqbuilder = IncomingRequestBuilder();
    reqbuilder.uri(Uri("example.com")).method("GET").version(version::v1_1);
    auto request = move(reqbuilder).build();
    sockaddr_in funny { .sin_port = 0, .sin_addr = { 0 } };

    cgi::CgiServerForwardInfo csfi {
        .sockaddr = net::SocketAddr(funny),
        .server_name = "aaaa",
        .server_port = 1234,
    };

    EXPECT_ANY_THROW({
        auto builder = OutgoingResponseBuilder();
        auto response = builder
                            .status(status::OK)
                            .cgi(Cgi("DAFSDFASDFASDFASDFASDFASDFasdfASDFASDFASDFASDF", move(request), csfi))
                            .build();

        string result = rr_read_to_string_ioenabled(ioe, move(response));
    });
}

TEST(ResponseReaderTests, rr_cgi_body_reqbody)
{
    // fake ioev
    auto ioe = ioruntime::IoEventHandler();
    ioruntime::GlobalIoEventHandler::set(&ioe);
    auto to = ioruntime::TimeoutEventHandler();
    ioruntime::GlobalTimeoutEventHandler::set(&to);

    // fake request data
    auto reqbody = vector<uint8_t>();
    reqbody.push_back('h');
    reqbody.push_back('e');
    reqbody.push_back('l');
    reqbody.push_back('l');
    reqbody.push_back('o');

    auto reqbuilder = IncomingRequestBuilder();
    reqbuilder.uri(Uri("example.com")).method("POST").version(version::v1_1).message(IncomingMessage(Headers(), IncomingBody(move(reqbody))));
    auto request = move(reqbuilder).build();
    sockaddr_in funny { .sin_port = 0, .sin_addr = { 0 } };

    cgi::CgiServerForwardInfo csfi {
        .sockaddr = net::SocketAddr(funny),
        .server_name = "aaaa",
        .server_port = 1234,
    };

    auto builder = OutgoingResponseBuilder();
    auto response = builder
                        .status(status::OK)
                        .cgi(Cgi("./tests/cgi_test/body_echo.lua", move(request), csfi))
                        .build();

    string result = rr_read_to_string_ioenabled(ioe, move(response));
    EXPECT_EQ(result, "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello");
}

}