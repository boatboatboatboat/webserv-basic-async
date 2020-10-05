//
// Created by boat on 10/1/20.
//

#include "Server.hpp"

http::TimeoutError::TimeoutError()
    : std::runtime_error("Timed out")
{
}