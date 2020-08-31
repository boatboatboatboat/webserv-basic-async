//
// Created by boat on 31-08-20.
//

#ifndef WEBSERV_FS_FILE_HPP
#define WEBSERV_FS_FILE_HPP

#include "../ioruntime/FileDescriptor.hpp"

namespace fs {

using ioruntime::FileDescriptor;

class File: public FileDescriptor {
public:
    File() = delete;
    static File open(std::string const& path);
    static File open_no_traversal(std::string const& path);
    static File from_raw_fd(int fd);
private:
    explicit File(FileDescriptor&& fd);
};

}

#endif //WEBSERV_FS_FILE_HPP
