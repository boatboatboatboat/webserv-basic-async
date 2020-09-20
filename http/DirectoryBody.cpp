//
// Created by boat on 9/8/20.
//

#include "DirectoryBody.hpp"

http::DirectoryBody::DirectoryBody(const std::string& str, std::string const& real_pathstr)
{
    std::string no_traversal = str;

    // we're just on unix, we don't have to check the million windows cases
    auto pos = no_traversal.find("../");
    while (pos != std::string::npos) {
        no_traversal.erase(pos, 3);
        pos = no_traversal.find("../");
    }

    while (no_traversal.ends_with("..")) {
        no_traversal.pop_back();
    }

    dir = opendir(no_traversal.c_str());
    if (dir == nullptr) {
        if (errno == ENOENT) {
            throw std::invalid_argument("No such file or directory");
        }
        throw std::system_error(errno, std::system_category());
    }
    path = no_traversal;
    while (path.ends_with('/'))
        path.pop_back();
    path += "/";

    real_path = real_pathstr;
    while (real_path.ends_with('/'))
        real_path.pop_back();
    real_path += "/";
}

// FIXME: pageend not written?
auto http::DirectoryBody::poll_read(char* buffer, size_t size, Waker&& waker) -> PollResult<ssize_t>
{
    auto written = std::min(size, cur.length());
    memcpy(buffer, cur.data(), written);
    cur.remove_prefix(written);
    if (cur.length() == 0) {
        switch (state) {
        case PageStart: {
            state = DirName;
            cur = real_path;
        } break;
        case DirName: {
            state = PageStart2;
            cur = PAGE_START2;
        } break;
        case PageStart2: {
            state = DirName2;
            cur = real_path;
        } break;
        case DirName2: {
            state = PageStart3;
            cur = PAGE_START3;
        } break;
        case PageStart3: {
            dirent = readdir(dir);
            if (dirent == nullptr) {
                state = PageEnd;
                cur = PAGE_END;
                break;
            }
            state = FileLinkStart;
            cur = FILE_LINK_START;
        } break;
        case FileLinkStart: {
            state = FileLinkPath;
            cur = real_path;
        } break;
        case FileLinkPath: {
            state = FileLinkName;
            cur = std::string_view(dirent->d_name, utils::strlen(dirent->d_name));
        } break;
        case FileLinkName: {
            state = FileLinkEnd;
            cur = FILE_LINK_END;
        } break;
        case FileLinkEnd: {
            state = FileName;
            cur = std::string_view(dirent->d_name, utils::strlen(dirent->d_name));
        } break;
        case FileName: {
            state = FileEnd;
            cur = FILE_END;
        } break;
        case FileEnd: {
            dirent = readdir(dir);
            if (dirent == nullptr) {
                state = PageEnd;
                cur = PAGE_END;
            } else {
                state = FileLinkStart;
                cur = FILE_LINK_START;
            }
        } break;
        case PageEnd: {
        } break;
        }
    }
    waker();
    return PollResult<ssize_t>::ready(written);
}

http::DirectoryBody::~DirectoryBody()
{
    closedir(dir);
}
