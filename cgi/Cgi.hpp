//
// Created by boat on 9/28/20.
//

#ifndef WEBSERV_CGI_CGI_HPP
#define WEBSERV_CGI_CGI_HPP

#include "../http/IncomingBody.hpp"
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

struct CgiServerForwardInfo {
    SocketAddr const& sockaddr;
    std::string_view server_name;
    unsigned short server_port;
};

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
    explicit Cgi(const string& path, Request&& req, CgiServerForwardInfo const& csfi);
    template <typename Lang>
    explicit Cgi(const string& path, Request&& req, CgiServerForwardInfo const& csfi, Lang&& language);
    ~Cgi() override;

    auto poll_success(Waker&& waker) -> bool;

    auto poll_write_body(Waker&& waker) -> PollResult<IoResult>;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
    auto poll_write(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    // methods
    void generate_env(Request const& req, CgiServerForwardInfo const& csfi);
    void create_pipe();
    auto fork_process() -> bool;
    void setup_redirection();
    void child_indicate_failure();
    void switch_external_process();
    template <typename Lang>
    void switch_internal_process(Lang&& language);
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

template <typename Lang>
void Cgi::switch_internal_process(Lang&& language)
{
    auto executable_start = _path.rfind('/') + 1;
    auto executable = _path.substr(executable_start);
    language.load_file(move(executable));
    language.load_environment(move(env));
    language.execute();
}

template <typename Lang>
Cgi::Cgi(const string& path, Request&& req, CgiServerForwardInfo const& csfi, Lang&& language)
    : _path(path)
    , _request(move(req))
{
    // don't verify
    generate_env(_request, csfi);
    create_pipe();
    if (fork_process()) {
        try {
            setup_redirection();
            switch_internal_process(std::forward<Lang>(language));
            // if failed abort
        } catch (...) {
            child_indicate_failure();
        }
    }
    env.clear();
}

}

#endif //WEBSERV_CGI_CGI_HPP
