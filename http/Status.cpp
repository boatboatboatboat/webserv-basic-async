//
// Created by boat on 8/27/20.
//

#include "Status.hpp"
#include "RfcConstants.hpp"

std::ostream& operator<<(std::ostream& os, http::Status const& status)
{
    os << status.code << http::SP << status.message;
    return os;
}
