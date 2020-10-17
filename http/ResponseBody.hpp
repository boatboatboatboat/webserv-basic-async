//
// Created by Djevayo Pattij on 10/2/20.
//

#ifndef WEBSERV_HTTP_RESPONSEBODY_HPP
#define WEBSERV_HTTP_RESPONSEBODY_HPP

#include "../cgi/Cgi.hpp"
#include "../ioruntime/IAsyncRead.hpp"

namespace http {

using cgi::Cgi;
using ioruntime::IAsyncRead;

class ResponseBody final : public IAsyncRead {
public:
    // special methods
    ResponseBody() = delete;
    ResponseBody(ResponseBody&&) noexcept;
    auto operator=(ResponseBody&&) noexcept -> ResponseBody&;
    ~ResponseBody() override;

    // ctors
    explicit ResponseBody(BoxPtr<ioruntime::IAsyncRead>&& reader);
    explicit ResponseBody(Cgi&& cgi);

    // methods
    [[nodiscard]] auto is_reader() const -> bool;
    [[nodiscard]] auto is_cgi() const -> bool;
    auto get_cgi() -> Cgi&;

    // implement: IAsyncRead
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<ioruntime::IoResult> override;

private:
    enum Tag {
        ReaderTag,
        CgiTag,
    } _tag;
    union {
        BoxPtr<ioruntime::IAsyncRead> _reader;
        Cgi _cgi;
    };
};

}

#endif //WEBSERV_HTTP_RESPONSEBODY_HPP
