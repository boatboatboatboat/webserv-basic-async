//
// Created by boat on 8/27/20.
//

#include "Version.hpp"

std::ostream& operator<<(std::ostream& os, http::Version const& version)
{
    os << version.version_string;
    return os;
}
