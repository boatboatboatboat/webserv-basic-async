//
// Created by boat on 08-10-20.
//

#ifndef WEBSERV_FS_PATH_HPP
#define WEBSERV_FS_PATH_HPP

#include <string>
#include "../option/optional.hpp"

namespace fs {

class Path {
public:
    Path() = delete;
    static auto path(std::string_view _path) -> Path;
    static auto no_traversal(std::string_view path) -> Path;
    [[nodiscard]] auto is_file() const -> bool;
    [[nodiscard]] auto is_directory() const -> bool;
    [[nodiscard]] auto is_weird() const -> bool;
    [[nodiscard]] auto is_executable() const -> bool;
    [[nodiscard]] auto get_extension() const -> option::optional<std::string> const&;
    [[nodiscard]] auto get_last_modification_time() const -> option::optional<time_t> const&;
    [[nodiscard]] auto exists() const -> bool;
private:
    explicit Path(std::string const& path);
    enum FileType {
        TypeFile,
        TypeDirectory,
        TypeWeird,
    } _file_type;
    bool _exists = false;
    bool _executable = false;
    option::optional<std::string> _file_extension;
    option::optional<time_t> _mtime;
};

}

#endif //WEBSERV_FS_PATH_HPP
