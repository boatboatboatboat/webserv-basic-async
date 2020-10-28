//
// Created by Djevayo Pattij on 10/2/20.
//

#ifndef WEBSERV_HTTP_OUTGOINGBODY_HPP
#define WEBSERV_HTTP_OUTGOINGBODY_HPP

#include "../cgi/Cgi.hpp"
#include "../ioruntime/IAsyncRead.hpp"

namespace http {

using cgi::Cgi;
using ioruntime::IAsyncRead;

class OutgoingBody final : public IAsyncRead {
public:
    // special methods
    OutgoingBody() = delete;
    OutgoingBody(OutgoingBody&&) noexcept;
    auto operator=(OutgoingBody&&) noexcept -> OutgoingBody&;
    OutgoingBody(OutgoingBody const&) = delete;
    auto operator=(OutgoingBody const&) -> OutgoingBody& = delete;
    ~OutgoingBody() override;

    // ctors
    explicit OutgoingBody(BoxPtr<ioruntime::IAsyncRead>&& reader);
    explicit OutgoingBody(Cgi&& cgi);

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

#endif //WEBSERV_HTTP_OUTGOINGBODY_HPP
