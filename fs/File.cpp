//
// Created by boat on 31-08-20.
//

#include "File.hpp"

#include <fcntl.h>

fs::File::File(FileDescriptor&& fd):
    FileDescriptor(std::move(fd))
{

}

fs::File fs::File::open_no_traversal(std::string const& path)
{
    std::string no_traversal = path;

    // we're just on unix, we don't have to check the million windows cases
    auto pos = no_traversal.find("../");
    while (pos != std::string::npos) {
        no_traversal.erase(pos, 3);
        pos = no_traversal.find("../");
    }

    int fd = ::open(path.c_str(), O_RDONLY);

    if (fd == -1) {
        throw std::runtime_error(strerror(errno));
    }

    return fs::File::from_raw_fd(fd);
}

fs::File fs::File::open(std::string const& path)
{
    int fd = ::open(path.c_str(), O_RDONLY);

    if (fd == -1) {
        throw std::runtime_error(strerror(errno));
    }

    return fs::File::from_raw_fd(fd);
}

fs::File fs::File::from_raw_fd(int fd)
{
    return fs::File(FileDescriptor(fd));
}
