//
// Created by boat on 9/28/20.
//

#ifndef WEBSERV_CGI_CGI_HPP
#define WEBSERV_CGI_CGI_HPP

#include "../http/StreamingHttpRequestParser.hpp"
#include "../ioruntime/FileDescriptor.hpp"
#include "../ioruntime/GlobalChildProcessHandler.hpp"
#include "../ioruntime/IAsyncRead.hpp"
#include <string>

namespace cgi {

using http::StreamingHttpRequest;
using ioruntime::FileDescriptor;
using ioruntime::GlobalChildProcessHandler;
using std::string;

class Cgi : public IAsyncRead {
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

    explicit Cgi(StreamingHttpRequest& req, const string& path)
        : _path(path)
    {
        (void)req;
        env.emplace_back("HELLO=world");
        create_pipe();
        fork_process();
        // check if child errored
        while (true) {
            char buffer[1];
            auto res = child_error_ipc->poll_read(buffer, 1, Waker::dead());

            if (res.is_ready()) {
                if (res.get() > 0) {
                    throw CgiError("execve failure");
                }
            } else {
                auto ran = process_ran->lock();
                if (*ran || child_stdout->can_read()) {
                    return;
                }
            }
        }
    }

    auto poll_read(char* buffer, size_t size, Waker&& waker) -> PollResult<ssize_t> override
    {
        GlobalChildProcessHandler::register_handler(pid, waker.boxed());

        return child_stdout->poll_read(buffer, size, std::move(waker));
    }

private:
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
        child_stdout = FileDescriptor(output_redirection_fds[0]);
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
        // setup stdout
        if (dup2(output_redirection_fds[1], STDOUT_FILENO) < 0) {
            child_indicate_failure();
        }
        close(STDIN_FILENO);
        close(output_redirection_fds[0]);
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
    pid_t pid;
    RcPtr<Mutex<bool>> process_ran = RcPtr(Mutex(false));
    optional<FileDescriptor> child_stdout;
    optional<FileDescriptor> child_error_ipc;
    vector<string> env;
    const string& _path;
};

}

#endif //WEBSERV_CGI_CGI_HPP
