//
// Created by Djevayo Pattij on 10/23/20.
//

#ifndef WEBSERV_UTILS_STRINGSTREAM_HPP
#define WEBSERV_UTILS_STRINGSTREAM_HPP

#include <string>

namespace utils {
class StringStream final {
public:
    StringStream() = default;
    explicit StringStream(std::string_view view);
    StringStream(StringStream const&) = default;
    auto operator=(StringStream const&) -> StringStream& = default;
    StringStream(StringStream&&) = default;
    auto operator=(StringStream&&) -> StringStream& = default;
    virtual ~StringStream() = default;

    auto str() -> std::string;
    auto operator<<(std::string_view view) -> StringStream&;
    auto operator<<(std::string const& view) -> StringStream&;
    auto operator<<(const char* view) -> StringStream&;
    auto operator<<(size_t view) -> StringStream&;
    auto operator<<(ssize_t view) -> StringStream&;
    auto operator<<(int view) -> StringStream&;
    auto operator<<(char view) -> StringStream&;

private:
    std::string _buffer;
};
}

#endif //WEBSERV_UTILS_STRINGSTREAM_HPP
