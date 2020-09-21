//
// Created by boat on 8/27/20.
//

#include "HttpVersion.hpp"

std::ostream& operator<<(std::ostream& os, http::HttpVersion const& version)
{
    os << version.version_string;
    return os;
}
