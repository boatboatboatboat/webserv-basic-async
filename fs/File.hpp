//
// Created by boat on 31-08-20.
//

#ifndef WEBSERV_FS_FILE_HPP
#define WEBSERV_FS_FILE_HPP

#include "../ioruntime/FileDescriptor.hpp"

namespace fs {

using ioruntime::FileDescriptor;

class FileError : public std::runtime_error {
public:
    FileError() = delete;
    explicit FileError(const char *w);
};

class FileNotFound : public FileError {
public:
    FileNotFound();
};

class FileIsDirectory : public FileError {
public:
    FileIsDirectory();
};

class File : public FileDescriptor {
public:
    File() = delete;
    static auto open(std::string const& path) -> File;
    static auto open_no_traversal(std::string const& path) -> File;
    static auto from_raw_fd(int fd) -> File;
    auto size() -> size_t;

private:
    explicit File(FileDescriptor&& fd);
};

}

#endif //WEBSERV_FS_FILE_HPP
