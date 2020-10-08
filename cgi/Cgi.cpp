//
// Created by boat on 9/28/20.
//

#include "Cgi.hpp"
#include "../constants/constants.hpp"

namespace cgi {

Cgi::CgiError::CgiError(const char* w)
    : std::runtime_error(w)
{
}

Cgi::PipeSetupFailed::PipeSetupFailed()

    : CgiError("CGI pipe setup failed")
{
}

Cgi::ForkFailed::ForkFailed()
    : CgiError("CGI fork failed")
{
}

Cgi::Cgi(const string& path, Request&& req, const SocketAddr& addr)
    : _path(path)
    , _request(move(req))
    , _body_span(const_cast<uint8_t*>(_request.get_body().data()), _request.get_body().size())
{
    generate_env(_request, addr);
    create_pipe();
    fork_process();
    env.clear();
}

auto Cgi::poll_success(Waker&& waker) -> bool
{
    GlobalChildProcessHandler::register_handler(pid, waker.boxed());

    uint8_t errbuf[1];
    auto res = child_error_ipc->poll_read(span(errbuf, 1), Waker(waker));

    if (res.is_ready()) {
        if (res.get().is_success()) {
            throw CgiError("execve failure");
        }
        return true;
    } else {
        auto ran = process_ran->lock();
        if (*ran || child_output->can_read()) {
            return true;
        } else {
            // just try to poll write the body and see if something happens
            poll_write_body(move(waker));
        }
    }
    return false;
}

auto Cgi::poll_write_body(Waker&& waker) -> PollResult<IoResult>
{
    if (child_input.has_value()) {
        auto poll_res = child_input->poll_write(_body_span, move(waker));
        if (poll_res.is_ready()) {
            auto res = poll_res.get();
            if (res.is_error()) {
                // probably a broken pipe or something
                // we can't do anything and we don't care either
                WARNPRINT("CGI body write failure");
            } else if (res.is_eof()) {
                // eof: destroy child_input!
                child_input = option::nullopt;
            } else if (res.is_success()) {
                _body_span.remove_prefix_inplace(res.get_bytes_read());
            }
        }
        return poll_res;
    } else {
        return PollResult<IoResult>::ready(IoResult::eof());
    }
}

auto Cgi::poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    GlobalChildProcessHandler::register_handler(pid, waker.boxed());
    return child_output->poll_read(buffer, std::move(waker));
}

auto Cgi::poll_write(const span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult>
{
    return child_input->poll_write(buffer, std::move(waker));
}

void Cgi::generate_env(Request const& req, SocketAddr const& addr)
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
    env.emplace_back(SERVER_SOFTWARE_METAVAR);
    // generate http header metavars
    for (auto& header : _request.get_headers()) {
        if (header.name == http::header::AUTHORIZATION || header.name == http::header::CONTENT_LENGTH
            || header.name == http::header::CONTENT_TYPE) {
            // remove recommended headers
            continue;
        }
        std::stringstream new_var;

        new_var << "HTTP_";
        for (auto c : header.name) {
            if (c == '-') {
                new_var << '_';
            } else {
                new_var << utils::to_uppercase(c);
            }
        }
        new_var << "=" << header.value;
        env.emplace_back(new_var.str());
    }
}

void Cgi::create_pipe()
{
    if (pipe(output_redirection_fds) < 0) {
        throw PipeSetupFailed();
    }
    if (pipe(input_redirection_fds) < 0) {
        close(output_redirection_fds[0]);
        close(output_redirection_fds[1]);
        throw PipeSetupFailed();
    }
    if (pipe(error_indication_fds) < 0) {
        close(output_redirection_fds[0]);
        close(input_redirection_fds[0]);
        close(output_redirection_fds[1]);
        close(input_redirection_fds[1]);
        throw PipeSetupFailed();
    }
    child_output = FileDescriptor(output_redirection_fds[0]);
    child_input = FileDescriptor(input_redirection_fds[1]);
    child_error_ipc = FileDescriptor(error_indication_fds[0]);
}

void Cgi::fork_process()
{
    pid = fork();

    if (pid < 0) {
        throw ForkFailed();
    } else if (pid == 0) {
        do_child_logic();
    } else {
        close(output_redirection_fds[1]);
        close(input_redirection_fds[0]);
        close(error_indication_fds[1]);
        // TODO: check if this is a race condition, and that the signal may be fired before the handler is created
        GlobalChildProcessHandler::register_handler(pid, BoxPtr<SetReadyFunctor>::make(RcPtr(process_ran)));
    }
}

void Cgi::do_child_logic()
{
    // chdir to script parent
    auto path_end = _path.rfind('/');
    try {
        auto path = _path.substr(0, path_end);
        if (chdir(path.c_str()) < 0) {
            throw std::exception();
        }
    } catch (...) {
        child_indicate_failure();
    }

    // redirect stdout
    if (dup2(output_redirection_fds[1], STDOUT_FILENO) < 0) {
        child_indicate_failure();
    }

    // redirect stdin
    if (dup2(input_redirection_fds[0], STDIN_FILENO) < 0) {
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

void Cgi::child_indicate_failure()
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

void Cgi::switch_process()
{
    auto executable_start = _path.rfind('/') + 1;
    auto executable = _path.substr(executable_start);

    const char* real_argv[2];
    real_argv[0] = executable.c_str();
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

}