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
    File(File&&) noexcept = default;
    File& operator=(File&&) noexcept = default;
    File(File const& other) = delete;
    auto operator=(File const& other) -> File& = delete;
    ~File() override;
    static auto open(std::string const& path) -> File;
    static auto open_no_traversal(std::string const& path) -> File;
    static auto create(std::string const& path) -> File;
    static auto create_no_traversal(std::string const& path) -> File;
    static auto from_raw_fd(int fd, bool temporary = false, std::string fname = "") -> File;
    static auto temporary() -> File;
    auto size() -> size_t;

private:
    explicit File(FileDescriptor&& fd, bool temporary = false, std::string tname = "");
    bool _file_is_temporary = false;
    std::string _handle;
};

}

#endif //WEBSERV_FS_FILE_HPP
