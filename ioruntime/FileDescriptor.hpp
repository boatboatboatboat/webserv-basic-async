//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_FUTURES_FILEDESCRIPTOR_HPP
#define WEBSERV_FUTURES_FILEDESCRIPTOR_HPP

#include "../boxed/boxed.hpp"
#include "../func/Functor.hpp"
#include "../func/SetReadyFunctor.hpp"
#include "IAsyncRead.hpp"
#include "IAsyncWrite.hpp"

using boxed::RcPtr;

namespace ioruntime {

class FileDescriptor : public IAsyncRead, public IAsyncWrite {
public:
    explicit FileDescriptor(int fd);
    FileDescriptor(FileDescriptor&& other) noexcept;
    auto operator=(FileDescriptor&& other) noexcept -> FileDescriptor&;
    ~FileDescriptor() override;
    virtual auto read(void* buffer, size_t size) -> ssize_t;
    virtual auto write(void const* buffer, size_t size) -> ssize_t;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
    auto poll_write(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;
    auto can_read() -> bool;
    auto can_write() -> bool;
    auto close() && -> int;
    [[nodiscard]] auto get_descriptor() const -> int;

protected:
    int descriptor;
    RcPtr<Mutex<bool>> ready_to_read = RcPtr(Mutex(false));
    RcPtr<Mutex<bool>> ready_to_write = RcPtr(Mutex(false));

private:
    FileDescriptor();
};

}

#endif //WEBSERV_FUTURES_FILEDESCRIPTOR_HPP
