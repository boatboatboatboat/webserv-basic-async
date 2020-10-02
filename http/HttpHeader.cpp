//
// Created by boat on 8/27/20.
//

#include "HttpHeader.hpp"

auto http::HttpHeader::operator==(const http::HttpHeader& other) const -> bool
{
    return (name.size() == other.name.size())
        && std::equal(name.begin(), name.end(), other.name.begin(), other.name.end(),
            [](char lhs, char rhs) {
                return (lhs == rhs) || ((lhs ^ 32) == rhs);
            });
}
