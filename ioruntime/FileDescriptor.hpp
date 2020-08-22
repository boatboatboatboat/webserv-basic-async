//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_FUTURES_FILEDESCRIPTOR_HPP
#define WEBSERV_FUTURES_FILEDESCRIPTOR_HPP

#include "../boxed/boxed.hpp"
#include "../func/Functor.hpp"
#include "IAsyncRead.hpp"
#include "IAsyncWrite.hpp"

using boxed::RcPtr;

namespace ioruntime {

class FileDescriptor : public IAsyncRead, public IAsyncWrite {
protected:
    class SetReadyFunctor : public Functor {
    public:
        explicit SetReadyFunctor(RcPtr<Mutex<bool>>&& cr_source);
        ~SetReadyFunctor() override = default;
        void operator()() override;

    private:
        RcPtr<Mutex<bool>> ready_mutex;
    };

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
