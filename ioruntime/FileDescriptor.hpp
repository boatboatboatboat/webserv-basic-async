//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_FUTURES_FILEDESCRIPTOR_HPP
#define WEBSERV_FUTURES_FILEDESCRIPTOR_HPP

#include "../boxed/boxed.hpp"
#include "../func/Functor.hpp"
#include "IAsyncRead.hpp"
#include "IAsyncWrite.hpp"
#include "../func/SetReadyFunctor.hpp"

using boxed::RcPtr;

namespace ioruntime {

class FileDescriptor : public IAsyncRead, public IAsyncWrite {
public:
    explicit FileDescriptor(int fd);
    static FileDescriptor uninitialized();
    FileDescriptor(FileDescriptor&& other) noexcept;
    virtual ~FileDescriptor();
    virtual ssize_t read(char* buffer, size_t size);
    virtual ssize_t write(const char* buffer, size_t size);
    PollResult<ssize_t> poll_read(char* buffer, size_t size, Waker&& waker) override;
    PollResult<ssize_t> poll_write(const char* buffer, size_t size, Waker&& waker) override;
    int close() &&;
    [[nodiscard]] int get_descriptor() const;

protected:
    int descriptor;
    RcPtr<Mutex<bool>> ready_to_read = RcPtr(Mutex(false));
    RcPtr<Mutex<bool>> ready_to_write = RcPtr(Mutex(false));

private:
    FileDescriptor();
};

}

#endif //WEBSERV_FUTURES_FILEDESCRIPTOR_HPP
