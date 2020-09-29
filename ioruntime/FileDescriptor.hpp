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
    ~FileDescriptor() override;
    static auto uninitialized() -> FileDescriptor;
    virtual auto read(char* buffer, size_t size) -> ssize_t;
    virtual auto write(const char* buffer, size_t size) -> ssize_t;
    auto poll_read(char* buffer, size_t size, Waker&& waker) -> PollResult<ssize_t> override;
    auto poll_write(const char* buffer, size_t size, Waker&& waker) -> PollResult<ssize_t> override;
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
