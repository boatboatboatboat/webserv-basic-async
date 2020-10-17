//
// Created by boat on 08-10-20.
//

#include "Path.hpp"
#include <sys/stat.h>
#include <unistd.h>

namespace fs {

Path::Path(std::string const& path)
{
    struct stat statbuf {
    };

    auto path_start = path.rfind('/');
    if (path_start == std::string::npos) {
        path_start = 0;
    }

    auto path_end = path.find('.', path_start);
    if (path_end != std::string::npos) {
        if (path.length() == path_end) {
            _file_extension = "";
        } else {
            _file_extension = path.substr(path_end + 1);
        }
    }

    int stat_res = ::stat(path.c_str(), &statbuf);

    if (stat_res != 0) {
        _exists = false;
    } else {
        if (S_ISREG(statbuf.st_mode)) {
            _file_type = TypeFile;
        } else if (S_ISDIR(statbuf.st_mode)) {
            _file_type = TypeDirectory;
        } else {
            _file_type = TypeWeird;
        }
        if (statbuf.st_mode & S_IXUSR) {
            _executable = true;
        }
        _mtime = statbuf.st_mtim.tv_sec;
        _exists = true;
    }
}

auto Path::is_file() const -> bool
{
    return _file_type == TypeFile;
}

auto Path::is_directory() const -> bool
{
    return _file_type == TypeDirectory;
}

auto Path::is_weird() const -> bool
{
    return _file_type == TypeWeird;
}

auto Path::path(std::string_view _path) -> Path
{
    return Path(std::string(_path));
}

auto Path::no_traversal(std::string_view path) -> Path
{
    std::string untraversed(path);

    // we're just on unix, we don't have to check the million windows cases
    auto pos = untraversed.find("../");
    while (pos != std::string::npos) {
        untraversed.erase(pos, 3);
        pos = untraversed.find("../");
    }

    return Path(untraversed);
}

auto Path::exists() const -> bool
{
    return _exists;
}

auto Path::is_executable() const -> bool
{
    return _executable;
}

auto Path::get_extension() const -> option::optional<std::string> const&
{
    return _file_extension;
}

auto Path::get_last_modification_time() const -> option::optional<time_t> const&
{
    return _mtime;
}

}