//
// Created by boat on 9/30/20.
//

#ifndef WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP
#define WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP

#include "../futures/IFuture.hpp"
#include "../futures/Waker.hpp"
#include "IAsyncRead.hpp"
#include "IAsyncWrite.hpp"
#include "../option/optional.hpp"

using futures::IFuture;
using futures::PollResult;
using futures::Waker;
using option::optional;

namespace ioruntime {

class IoCopyFuture: public IFuture<void> {
public:
    IoCopyFuture() = delete;
    explicit IoCopyFuture(IAsyncRead& reader, IAsyncWrite& writer);
    auto poll(Waker&& waker) -> PollResult<void> override;
    [[nodiscard]] auto get_bytes_written() const -> size_t;
    [[nodiscard]] auto has_written_bytes() const -> bool;
private:
    uint8_t _buffer[8192];
    optional<span<uint8_t>> _span;
    size_t _bytes_written = 0;
    enum State {
        Reading,
        Writing
    } state = Reading;
    IAsyncRead& _reader;
    IAsyncWrite& _writer;
};

}

#endif //WEBSERV_IORUNTIME_IOCOPYFUTURE_HPP
