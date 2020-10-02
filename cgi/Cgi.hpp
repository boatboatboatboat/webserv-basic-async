//
// Created by boat on 9/28/20.
//

#ifndef WEBSERV_CGI_CGI_HPP
#define WEBSERV_CGI_CGI_HPP

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
using std::string;

class Cgi : public IAsyncRead, public IAsyncWrite {
public:
    class CgiError : public std::runtime_error {
    public:
        explicit CgiError(const char* w)
            : std::runtime_error(w)
        {
        }
    };
    class PipeSetupFailed : public CgiError {
    public:
        PipeSetupFailed()
            : CgiError("CGI pipe setup failed")
        {
        }
    };

    class ForkFailed : public CgiError {
    public:
        ForkFailed()
            : CgiError("CGI fork failed")
        {
        }
    };

    explicit Cgi(
        const string& path,
        Request const& req,
        SocketAddr const& addr)
        : _path(path)
    {
        generate_env(req, addr);
        create_pipe();
        fork_process();
        // check if child errored
        // basically, check if something has been written to the error pipe
        // if it has been, throw an error
        // otherwise, check if the process has finished and/or something has been written to the child's stdout
        //
        // note: a CGI script that just loops infinitely without printing anything will 'hang' the server
        // there is however no other way to check (within the limits of the subject) whether the CGI script succeeded
        // it is NOT possible to add a timeout to the CGI script, in case it's a stream (think M-JPEG streaming scripts)
        // or something like an expensive and/or slow operation (think drive accesses on a busy drive)
        /*
        while (true) {
            char buffer[1];
            auto res = child_error_ipc->poll_read(buffer, 1, Waker::dead());

            if (res.is_ready()) {
                if (res.get() > 0) {
                    throw CgiError("execve failure");
                }
            } else {
                auto ran = process_ran->lock();
                if (*ran || child_output->can_read()) {
                    return;
                }
            }
        }
        */
    }

    auto poll_success(Waker&& waker) -> bool
    {
        GlobalChildProcessHandler::register_handler(pid, waker.boxed());

        uint8_t errbuf[1];
        auto res = child_error_ipc->poll_read(span(errbuf, 1), std::move(waker));

        if (res.is_ready()) {
            if (res.get().is_success()) {
                throw CgiError("execve failure");
            }
        } else {
            auto ran = process_ran->lock();
            if (*ran || child_output->can_read()) {
                return true;
            }
        }
        return false;
    }

    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override
    {
        GlobalChildProcessHandler::register_handler(pid, waker.boxed());
        if (!child_succeeded) {
            uint8_t errbuf[1];
            auto res = child_error_ipc->poll_read(span(errbuf, 1), Waker(waker));

            if (res.is_ready()) {
                if (res.get().is_success()) {
                    throw CgiError("execve failure");
                }
            } else {
                auto ran = process_ran->lock();
                if (*ran || child_output->can_read()) {
                    child_succeeded = true;
                    return child_input->poll_read(buffer, std::move(waker));
                }
            }
        }
        return child_output->poll_read(buffer, std::move(waker));
    }

    auto poll_write(span<uint8_t> const buffer, Waker&& waker) -> PollResult<IoResult> override
    {
        if (!child_succeeded) {
            uint8_t errbuf[1];
            auto res = child_error_ipc->poll_read(span(errbuf, 1), Waker(waker));

            if (res.is_ready()) {
                if (res.get().is_success()) {
                    throw CgiError("execve failure");
                }
            } else {
                auto ran = process_ran->lock();
                if (*ran || child_output->can_read()) {
                    child_succeeded = true;
                    return child_input->poll_write(buffer, std::move(waker));
                }
            }
            return PollResult<IoResult>::pending();
        }
        return child_input->poll_write(buffer, std::move(waker));
    }

private:
    void generate_env(Request const& req, SocketAddr const& addr)
    {
        // TODO: set auth_type metavar
        // Set CONTENT_LENGTH metavar
        {
            auto cl = req.get_header("Content-Length");

            // FIXME: all requests have bodies - this needs to be determined in a better way
            if (!req.get_body().empty()) {
                std::stringstream var;
                var << "CONTENT_LENGTH=" << req.get_body().size();
                env.push_back(var.str());
            }
        }
        // Set CONTENT_TYPE metavar
        {
            auto cl = req.get_header("Content-Type");

            // It should only be set if there's a message body.
            if (cl.has_value()) {
                std::string var("CONTENT_TYPE=");
                var += *cl;
                env.push_back(std::move(var));
            }
        }
        // Set GATEWAY_INTERFACE metavar
        env.emplace_back("GATEWAY_INTERFACE=CGI/1.1");
        // Set PATH_INFO metavar
        {
            std::string builder("PATH_INFO=");
            auto pqf = req.get_uri().get_pqf();

            if (pqf.has_value()) {
                auto pv = pqf->get_path_escaped();

                if (pv.has_value()) {
                    builder += *pv;
                }
            }
            env.push_back(std::move(builder));
        }
        // Set PATH_TRANSLATED metavar
        {
            std::string builder("PATH_TRANSLATED=");
            // FIXME: empty paths are not handled
            builder += _path.substr(0, _path.rfind('/'));
            auto pqf = req.get_uri().get_pqf();

            if (pqf.has_value()) {
                auto pv = pqf->get_path_escaped();

                if (pv.has_value()) {
                    builder += *pv;
                }
            }
            env.push_back(std::move(builder));
        }
        // Set QUERY_STRING metavar
        {
            std::string qs("QUERY_STRING=");

            auto pqf = req.get_uri().get_pqf();
            if (pqf.has_value()) {
                auto query = pqf->get_query();

                if (query.has_value()) {
                    qs += *query;
                }
            }
            env.push_back(std::move(qs));
        }
        // Set REMOTE_ADDR metavar
        {
            std::stringstream ra;

            ra << "REMOTE_ADDR=";
            if (addr.is_v4()) {
                ra << net::Ipv4Address(addr.get_v4());
            } else if (addr.is_v6()) {
                ra << net::Ipv6Address(addr.get_v6());
            }
            env.push_back(ra.str());
        }
        // Set REMOTE_IDENT metavar
        {
            // RFC 3875-4.1.10
            // The server may choose not to support
            // this feature, or not to request the data for efficiency reasons, or
            // not to return available identity data.
            env.emplace_back("REMOTE_IDENT=");
        }
        // TODO: REMOTE_USER metavar
        // Set REQUEST_METHOD metavar
        {
            std::string rm("REQUEST_METHOD=");

            rm += req.get_method();
            env.push_back(std::move(rm));
        }
        // Set REQUEST_URI metavar
        {
            // REQUEST_URI is actually a SIP CGI metavar,
            // and not a HTTP CGI.
            // There is no defined behaviour for this value in HTTP CGI,
            // so let's just return the uri
            //
            // NOTE: the SIP CGI definition for REQUEST_URI is absoluteURI.
            // FIXME: the URI is not in absolute form
            std::stringstream uri;
            uri << "REQUEST_URI=";
            auto req_uri = req.get_uri();
            if (req_uri.get_scheme().has_value()) {
                uri << req_uri.get_scheme().value().get() << "://";
            }
            if (req_uri.get_authority().has_value()) {
                auto authority = req_uri.get_authority();
                if (authority->get_userinfo().has_value()) {
                    uri << authority->get_userinfo().value() << "@";
                }
                uri << authority->get_host();
                if (authority->get_port().has_value()) {
                    uri << ":" << authority->get_port().value();
                }
            }
            if (req_uri.get_pqf().has_value()) {
                auto pqf = req_uri.get_pqf().value();
                if (pqf.get_path().has_value()) {
                    uri << pqf.get_path().value();
                }
                if (pqf.get_query().has_value()) {
                    uri << "?" << pqf.get_query().value();
                }
                if (pqf.get_fragment().has_value()) {
                    uri << pqf.get_fragment().value();
                }
            }
            env.emplace_back(uri.str());
        }
        // Set SCRIPT_NAME metavar
        {
            std::string sn("SCRIPT_NAME=");

            sn += _path;
            env.push_back(std::move(sn));
        }
        // TODO: SERVER_NAME metavar
        // TODO: SERVER_PORT metavar
        // Set SERVER_PROTOCOL metavar
        // Let's just hardcode these headers
        env.emplace_back("SERVER_PROTOCOL=HTTP/1.1");
        env.emplace_back("SERVER_SOFTWARE=BroServ/latest");
    }

    void create_pipe()
    {
        if (pipe(output_redirection_fds) < 0) {
            throw PipeSetupFailed();
        }
        if (pipe(error_indication_fds) < 0) {
            close(output_redirection_fds[0]);
            close(output_redirection_fds[1]);
            throw PipeSetupFailed();
        }
        child_output = FileDescriptor(output_redirection_fds[0]);
        child_input = FileDescriptor(output_redirection_fds[1]);
        child_error_ipc = FileDescriptor(error_indication_fds[0]);
    }

    void fork_process()
    {
        pid = fork();

        if (pid < 0) {
            throw ForkFailed();
        } else if (pid == 0) {
            do_child_logic();
        } else {
            close(output_redirection_fds[1]);
            close(error_indication_fds[1]);
            // TODO: check if this is a race condition, and that the signal may be fired before the handler is created
            GlobalChildProcessHandler::register_handler(pid, BoxPtr<SetReadyFunctor>::make(RcPtr(process_ran)));
        }
    }

    void do_child_logic()
    {
        // redirect stdout
        if (dup2(output_redirection_fds[1], STDOUT_FILENO) < 0) {
            child_indicate_failure();
        }
        // redirect stdin
        if (dup2(output_redirection_fds[0], STDIN_FILENO) < 0) {
            child_indicate_failure();
        }
        close(error_indication_fds[0]);
        try {
            switch_process();
            // if failed abort
        } catch (...) {
            child_indicate_failure();
        }
    }

    void child_indicate_failure()
    {
        // We need a new select loop because our previous select loop might not exist,
        // because we possibly forked from a different thread than the Runtime thread

        // For now, just repeatedly try and write to the error pipe.
        // If there's >0 characters written, the process will be marked as 'errored'
        fd_set me;
        for (;;) {
            FD_ZERO(&me);
            FD_SET(error_indication_fds[1], &me);

            int res = select(error_indication_fds[1] + 1, NULL, &me, NULL, NULL);
            if (res > 0) {
                if (write(error_indication_fds[1], "F", 1) > 0) {
                    exit(1);
                }
            }
        }
    }

    void switch_process()
    {
        const char* real_argv[2];
        real_argv[0] = _path.c_str();
        real_argv[1] = nullptr;
        char* const* argv = const_cast<char* const*>(real_argv);

        vector<const char*> envp;
        for (auto& str : env) {
            envp.push_back(str.c_str());
        }
        envp.push_back(nullptr);

        auto switch_result = execve(argv[0], reinterpret_cast<char* const*>(argv), const_cast<char* const*>(envp.data()));
        if (switch_result < 0) {
            throw std::runtime_error("execve failed");
        }
    }

    int output_redirection_fds[2];
    int error_indication_fds[2];
    bool child_succeeded = false;
    pid_t pid;
    RcPtr<Mutex<bool>> process_ran = RcPtr(Mutex(false));
    optional<FileDescriptor> child_input;
    optional<FileDescriptor> child_output;
    optional<FileDescriptor> child_error_ipc;
    vector<string> env;
    const string& _path;
};

}

#endif //WEBSERV_CGI_CGI_HPP
