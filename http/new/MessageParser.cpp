//
// Created by boat on 19-10-20.
//

#include "MessageParser.hpp"

namespace http {

MessageParser::MessageParser(
    ioruntime::CharacterStream& reader,
    size_t buffer_limit,
    size_t body_limit)
    : _reader(reader)
    , _buffer_limit(buffer_limit)
    , _body_limit(body_limit)
{
}

}
