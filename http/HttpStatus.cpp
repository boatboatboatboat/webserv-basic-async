//
// Created by boat on 8/27/20.
//

#include "HttpStatus.hpp"
#include "HttpRfcConstants.hpp"

std::ostream& operator<<(std::ostream& os, http::HttpStatus const& status)
{
    os << status.code << http::HTTP_SP << status.message;
    return os;
}
