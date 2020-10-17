//
// Created by boat on 31-08-20.
//

#include "File.hpp"

#include "Path.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

fs::File::File(FileDescriptor&& fd, bool temporary, std::string fname)
    : FileDescriptor(std::move(fd))
    , _file_is_temporary(temporary)
    , _handle(fname)
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

    int fd = ::open(no_traversal.c_str(), O_RDONLY);

    if (fd == -1) {
        if (errno == ENOENT) {
            throw fs::FileNotFound();
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
            throw fs::FileNotFound();
        }
        throw std::system_error(errno, std::system_category());
    }

    return fs::File::from_raw_fd(fd);
}

auto fs::File::from_raw_fd(int fd, bool temporary, std::string fname) -> fs::File
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
        throw fs::FileIsDirectory();
    }

    return fs::File(FileDescriptor(fd), temporary, fname);
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

auto fs::File::create(const std::string& path) -> fs::File
{
    int fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC);

    if (fd == -1) {
        if (errno == ENOENT) {
            throw fs::FileNotFound();
        }
        throw std::system_error(errno, std::system_category());
    }

    return fs::File::from_raw_fd(fd);
}

auto fs::File::create_no_traversal(std::string const& path) -> fs::File
{
    std::string no_traversal = path;

    // we're just on unix, we don't have to check the million windows cases
    auto pos = no_traversal.find("../");
    while (pos != std::string::npos) {
        no_traversal.erase(pos, 3);
        pos = no_traversal.find("../");
    }

    int fd = ::open(no_traversal.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0646);

    if (fd == -1) {
        if (errno == ENOENT) {
            throw fs::FileNotFound();
        }
        DBGPRINT(strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return fs::File::from_raw_fd(fd);
}

auto fs::File::temporary() -> fs::File
{
    std::string tf_name;
    struct timeval tv {
    };

    for (;;) {
        gettimeofday(&tv, NULL);
        tf_name = "/tmp/tmp_" + std::to_string(tv.tv_usec);
        auto path = Path::path(tf_name);

        if (!path.exists()) {
            int fd = ::open(tf_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);

            if (fd == -1) {
                if (errno == EEXIST) {
                    continue;
                }
                DBGPRINT(strerror(errno));
                throw std::system_error(errno, std::system_category());
            }

            return fs::File::from_raw_fd(fd, true, tf_name);
        }
    }
}

fs::File::~File()
{
    if (descriptor > -1 && _file_is_temporary) {
        unlink(_handle.c_str());
    }
}

fs::FileNotFound::FileNotFound()
    : FileError("File not found")
{
}

fs::FileIsDirectory::FileIsDirectory()
    : FileError("File is directory")
{
}

fs::FileError::FileError(const char* w)
    : std::runtime_error(w)
{
}
