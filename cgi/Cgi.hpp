//
// Created by boat on 9/28/20.
//

#ifndef WEBSERV_CGI_CGI_HPP
#define WEBSERV_CGI_CGI_HPP

#include "../http/RequestBody.hpp"
#include "../http/RequestParser.hpp"
#include "../ioruntime/FileDescriptor.hpp"
#include "../ioruntime/GlobalChildProcessHandler.hpp"
#include "../ioruntime/IAsyncRead.hpp"
#include "../net/IpAddress.hpp"
#include "../net/SocketAddr.hpp"
#include <string>

namespace cgi {

using http::Request;
using ioruntime::FileDescriptor;
using ioruntime::GlobalChildProcessHandler;
using ioruntime::IAsyncWrite;
using ioruntime::IoResult;
using net::SocketAddr;
using std::move;
using std::string;

class Cgi : public IAsyncRead, public IAsyncWrite {
public:
    class CgiError : public std::runtime_error {
    public:
        explicit CgiError(const char* w);
    };
    class PipeSetupFailed : public CgiError {
    public:
        PipeSetupFailed();
    };
    class ForkFailed : public CgiError {
    public:
        ForkFailed();
    };

    Cgi(Cgi&&) noexcept = default;
    explicit Cgi(const string& path, Request&& req, SocketAddr const& addr);
    ~Cgi() override;

    auto poll_success(Waker&& waker) -> bool;

    auto poll_write_body(Waker&& waker) -> PollResult<IoResult>;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
    auto poll_write(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    // methods
    void generate_env(Request const& req, SocketAddr const& addr);
    void create_pipe();
    void fork_process();
    void do_child_logic();
    void child_indicate_failure();
    void switch_process();
    void verify_executable();
    // members
    int output_redirection_fds[2];
    int input_redirection_fds[2];
    int error_indication_fds[2];
    pid_t pid;
    RcPtr<Mutex<bool>> process_ran = RcPtr(Mutex(false));
    optional<FileDescriptor> child_input;
    optional<FileDescriptor> child_output;
    optional<FileDescriptor> child_error_ipc;
    vector<string> env;
    string const& _path;
    Request _request;
    optional<ioruntime::RefIoCopyFuture> _ricf;
};

}

#endif //WEBSERV_CGI_CGI_HPP
