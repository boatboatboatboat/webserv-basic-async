//
// Created by boat on 31-08-20.
//

#include "File.hpp"

#include <fcntl.h>
#include <sys/stat.h>

fs::File::File(FileDescriptor&& fd)
    : FileDescriptor(std::move(fd))
{
}

auto fs::File::open_no_traversal(std::string const& path) -> fs::File
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
        if (errno == ENOENT) {
            throw FileNotFound();
        }
        DBGPRINT(strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return fs::File::from_raw_fd(fd);
}

auto fs::File::open(std::string const& path) -> fs::File
{
    int fd = ::open(path.c_str(), O_RDONLY);

    if (fd == -1) {
        if (errno == ENOENT) {
            throw FileNotFound();
        }
        throw std::system_error(errno, std::system_category());
    }

    return fs::File::from_raw_fd(fd);
}

auto fs::File::from_raw_fd(int fd) -> fs::File
{
    struct stat sbuf {
    };

    if (fstat(fd, &sbuf)) {
        ::close(fd);
        throw std::system_error(errno, std::system_category());
    }

    // this is a file class
    // we only accept files
    if (!S_ISREG(sbuf.st_mode)) {
        ::close(fd);
        throw std::invalid_argument("Is directory");
    }

    return fs::File(FileDescriptor(fd));
}

auto fs::File::size() -> size_t
{
    struct stat sbuf {
    };

    if (fstat(get_descriptor(), &sbuf)) {
        throw std::system_error(errno, std::system_category());
    }

    return sbuf.st_size;
}

fs::File::FileNotFound::FileNotFound()
    : std::runtime_error("File not found")
{
}
