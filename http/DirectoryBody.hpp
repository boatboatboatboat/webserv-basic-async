//
// Created by boat on 9/8/20.
//

#ifndef WEBSERV_DIRECTORYBODY_HPP
#define WEBSERV_DIRECTORYBODY_HPP

#include "../ioruntime/IAsyncRead.hpp"
#include <dirent.h>
#include <string>

using std::string_view;
using ioruntime::IoResult;

namespace http {

class DirectoryBody : public ioruntime::IAsyncRead {
public:
    explicit DirectoryBody(string_view str, string_view real_pathstr);
    ~DirectoryBody() override;
    auto poll_read(span<uint8_t> buffer, Waker&& waker) -> PollResult<IoResult> override;

private:
    inline static constexpr std::string_view PAGE_START = "<html><head><title>";
    inline static constexpr std::string_view PAGE_START2 = "</title></head><body><h1>Index of ";
    inline static constexpr std::string_view PAGE_START3 = "</h1><hr/>";
    inline static constexpr std::string_view FILE_LINK_START = "<a href=\"";
    inline static constexpr std::string_view FILE_LINK_END = "\">";
    inline static constexpr std::string_view FILE_END = "</a><br/>";
    inline static constexpr std::string_view PAGE_END = "</body></html>";
    enum State {
        PageStart,
        DirName,
        PageStart2,
        DirName2,
        PageStart3,
        FileLinkStart,
        FileLinkPath,
        FileLinkName,
        FileLinkEnd,
        FileName,
        FileEnd,
        PageEnd
    } state
        = PageStart;
    DIR* dir;
    std::string path;
    std::string real_path;
    dirent* dirent;
    std::string_view cur = PAGE_START;
};

}

#endif //WEBSERV_DIRECTORYBODY_HPP
