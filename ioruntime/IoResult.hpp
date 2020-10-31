//
// Created by boat on 10/1/20.
//

#ifndef WEBSERV_IORUNTIME_IORESULT_HPP
#define WEBSERV_IORUNTIME_IORESULT_HPP

#include <cstdio>

namespace ioruntime {

class IoResult {
public:
    // special methods
    IoResult() = delete;
    ~IoResult() = default;

    // ctors

    // Construct a IoResult from a read/write result
    IoResult(ssize_t posix_io_result);
    [[nodiscard]] static auto eof() -> IoResult;
    [[nodiscard]] static auto error() -> IoResult;
    [[nodiscard]] static auto success(size_t bytes_read) -> IoResult;

    // methods
    [[nodiscard]] auto is_error() const -> bool;
    [[nodiscard]] auto is_eof() const -> bool;
    [[nodiscard]] auto is_success() const -> bool;
    [[nodiscard]] auto get_bytes_read() const -> size_t;

private:
    enum Tag {
        Error,
        Eof,
        Success,
    } _tag;
    explicit IoResult(Tag tag);
    IoResult(Tag tag, size_t bytes_read);
    size_t _bytes_read;
};

}

#endif //WEBSERV_IORUNTIME_IORESULT_HPP
