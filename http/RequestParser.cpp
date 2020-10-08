//
// Created by boat on 13-09-20.
//

#include "RequestParser.hpp"

using std::move;

namespace http {

RequestParser::ParserError::ParserError(const char* w)
    : std::runtime_error(w)
{
}

RequestParser::RequestUriExceededBuffer::RequestUriExceededBuffer()
    : ParserError("Request URI exceeded buffer limit")
{
}

RequestParser::BodyExceededLimit::BodyExceededLimit()
    : ParserError("Body exceeded limit")
{
}

RequestParser::GenericExceededBuffer::GenericExceededBuffer()
    : ParserError("Exceeded buffer limit")
{
}

RequestParser::InvalidMethod::InvalidMethod()
    : ParserError("Invalid HTTP method")
{
}

RequestParser::InvalidVersion::InvalidVersion()
    : ParserError("Invalid HTTP version")
{
}

RequestParser::MalformedRequest::MalformedRequest()
    : ParserError("Bad HTTP request")
{
}

RequestParser::UnexpectedEof::UnexpectedEof()
    : ParserError("Unexpected end of request")
{
}

RequestParser::UndeterminedLength::UndeterminedLength()
    : ParserError("Undetermined length of body")
{
}

RequestParser::BadTransferEncoding::BadTransferEncoding()
    : ParserError("Bad transfer encoding")
{
}

RequestParser::RequestParser(IAsyncRead& source, size_t buffer_limit, size_t body_limit)
    : buffer_limit(buffer_limit)
    , body_limit(body_limit)
    , source(source)
{
}

RequestParser::RequestParser(RequestParser&& other) noexcept
    : character_head(other.character_head)
    , character_max(other.character_max)
    , buffer_limit(other.buffer_limit)
    , body_limit(other.body_limit)
    , source(other.source)
    , buffer(move(other.buffer))
    , builder(move(other.builder))
{
    utils::memcpy(character_buffer, other.character_buffer);
    other.moved = true;
}

RequestParser::RequestParser(RequestParser&& other, IAsyncRead& new_stream) noexcept
    : character_head(other.character_head)
    , character_max(other.character_max)
    , buffer_limit(other.buffer_limit)
    , body_limit(other.body_limit)
    , source(new_stream)
    , buffer(move(other.buffer))
    , builder(move(other.builder))
{
    utils::memcpy(character_buffer, other.character_buffer);
    other.moved = true;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
auto RequestParser::poll(Waker&& waker) -> PollResult<Request>
{
    if (moved)
        return PollResult<Request>::pending();
    auto poll_result = poll_character(Waker(waker));
    using Status = StreamPollResult<uint8_t>::Status;

    switch (poll_result.get_status()) {
        case Status::Ready: {
            auto result = poll_result.get();
            auto matched = false;
            switch (state) {
                case ReadMethod:
                case ReadUri: {
                    matched = result == ' ';
                } break;
                case ReadVersion:
                case ChunkedBodySize:
                case HeaderLine: {
                    matched = result == '\n' && !buffer.empty() && buffer.back() == '\r';
                    if (matched) {
                        buffer.pop_back();
                    }
                } break;
                case ChunkedBodyData:
                case Body: {
                    matched = true;
                } break;
            }
            if (state == Body && buffer.size() == body_limit) {
                throw BodyExceededLimit();
            } else if (buffer.size() == buffer_limit) {
                if (state == ReadUri) {
                    throw RequestUriExceededBuffer();
                } else {
                    throw GenericExceededBuffer();
                }
            }
            if (!matched || state == Body || state == ChunkedBodyData) {
                buffer.push_back(result);
            }
            if (matched) {
                string_view str_in_buf(reinterpret_cast<const char*>(buffer.data()), buffer.size());
                switch (state) {
                    case ReadMethod: {
                        auto method = get_method(str_in_buf);
                        if (method.has_value()) {
                            current_method = *method;
                            builder.method(string_view(*method));
                        } else {
                            throw InvalidMethod();
                        }
                        buffer.clear();
                        state = ReadUri;
                    } break;
                    case ReadUri: {
                        auto uri = Uri(str_in_buf);
                        builder.uri(move(uri));
                        buffer.clear();
                        state = ReadVersion;
                    } break;
                    case ReadVersion: {
                        if (str_in_buf == version::v1_0.version_string) {
                            builder.version(version::v1_0);
                        } else if (str_in_buf == version::v1_1.version_string) {
                            is_http_1_1 = true;
                            builder.version(version::v1_1);
                        } else {
                            throw InvalidVersion();
                        }
                        buffer.clear();
                        state = HeaderLine;
                    } break;
                    case HeaderLine: {
                        if (str_in_buf.empty()) {
                            if (is_http_1_1 && !host_set) {
                                // RFC 7230 5.4.
                                // HTTP/1.1 must have Host header field
                                // otherwise, it is invalid
                                throw MalformedRequest();
                            }
                            if (!method_has_body(current_method)) {
                                return PollResult<Request>::ready(move(builder).build());
                            }
                            if (content_length.has_value()) {
                                state = Body;
                            } else if (chunked) {
                                state = ChunkedBodySize;
                            } else {
                                throw UndeterminedLength();
                            }
                        } else {
                            auto separator = str_in_buf.find(':');

                            if (separator == string_view::npos) {
                                throw MalformedRequest();
                            }

                            auto field_name = str_in_buf.substr(0, separator);
                            auto field_value = str_in_buf.substr(separator + 1);

                            // validate field_name
                            if (field_name.empty()) {
                                // field name must at least have 1 tchar
                                throw MalformedRequest();
                            }
                            for (auto c : field_name) {
                                // field name = token = 1*tchar
                                if (!parser_utils::is_tchar(c)) {
                                    // not a tchar = invalid request
                                    throw MalformedRequest();
                                }
                            }

                            // trim OWS from header value
                            while (!field_value.empty() && (field_value.front() == ' ' || field_value.front() == '\t')) {
                                field_value.remove_prefix(1);
                            }
                            while (!field_value.empty() && (field_value.back() == ' ' || field_value.back() == '\t')) {
                                field_value.remove_suffix(1);
                            }

                            if (utils::str_eq_case_insensitive(field_name, header::CONTENT_LENGTH)) {
                                auto length = utils::string_to_uint64(field_value);

                                if (length.has_value()) {
                                    content_length = *length;
                                } else {
                                    throw MalformedRequest();
                                }
                            } else if (utils::str_eq_case_insensitive(field_name, header::TRANSFER_ENCODING)) {
                                if (field_value != "chunked") {
                                    throw BadTransferEncoding();
                                }
                                chunked = true;
                            } else if (utils::str_eq_case_insensitive(field_name, header::HOST)) {
                                if (host_set) {
                                    // RFC 7230 5.4.
                                    // if there's more than 1 Host header it's an invalid request
                                    throw MalformedRequest();
                                }
                                host_set = true;
                            }

                            builder.header(string(field_name), string(field_value));
                        }
                        buffer.clear();
                    } break;
                    case Body: {
                        if (buffer.size() == *content_length) {
                            builder.body(move(buffer));
                            return PollResult<Request>::ready(move(builder).build());
                        }
                    } break;
                    case ChunkedBodySize: {
                        // basically, a size can be something like
                        // FF;12=34;56=78
                        // however we are allowed to ignore the extensions,
                        // so we're just going to ignore everything after the first ';'.
                        // however, we still have to check if the characters are valid
                        // because we need to send a 400 if that isn't the case

                        auto first_separator = str_in_buf.find(';');
                        if (first_separator == string_view::npos) {
                            first_separator = str_in_buf.length();
                        } else {
                            for (auto& c : str_in_buf.substr(first_separator, str_in_buf.length())) {
                                if (!parser_utils::is_tchar(c) && c != ';') {
                                    throw MalformedRequest();
                                }
                            }
                        }

                        auto decoded_size = utils::hexstring_to_uint64(str_in_buf.substr(0, first_separator));
                        if (decoded_size.has_value()) {
                            last_chunk_size = decoded_size.value();
                            if (decoded_size.value() == 0) {
                                builder.body(move(*decoded_body));
                                return PollResult<Request>::ready(move(builder).build());
                            }
                            // check if this would exceed the limit
                            if (decoded_body.has_value()) {
                                if ((decoded_body->size() + *decoded_size) > body_limit) {
                                    throw BodyExceededLimit();
                                }
                            } else if (*decoded_size > body_limit) {
                                throw BodyExceededLimit();
                            }
                            buffer.clear();
                            state = ChunkedBodyData;
                        } else {
                            throw MalformedRequest();
                        }
                    } break;
                    case ChunkedBodyData: {
                        if (buffer.size() == last_chunk_size + 2) {
                            // check body terminator
                            if (buffer.back() != '\n') {
                                throw MalformedRequest();
                            }
                            buffer.pop_back();
                            if (buffer.back() != '\r') {
                                throw MalformedRequest();
                            }
                            buffer.pop_back();
                            // initialize decoded_body
                            if (!decoded_body.has_value()) {
                                decoded_body = vector<uint8_t>();
                            }
                            // check if concatenation would exceed the limit
                            if ((decoded_body->size() + buffer.size()) > body_limit) {
                                throw BodyExceededLimit();
                            }
                            // append buffer to body
                            decoded_body->insert(decoded_body->end(), buffer.begin(), buffer.end());
                            // reset the buffer
                            buffer.clear();
                            // switch back to size
                            state = ChunkedBodySize;
                        }
                    } break;
                }
            }
            return poll(move(waker));
        } break;
        case Status::Pending: {
            return PollResult<Request>::pending();
        } break;
        case Status::Finished: {
            // All EOFs are unexpected, we want either TCE or CL
            throw UnexpectedEof();
        } break;
    }
};
#pragma clang diagnostic pop

auto RequestParser::poll_character(Waker&& waker) -> StreamPollResult<uint8_t>
{
    if (character_head == 0) {
        auto poll_result = source.poll_read(span(character_buffer, sizeof(character_buffer)), move(waker));

        if (poll_result.is_ready()) {
            auto result = poll_result.get();
            if (result.is_error()) {
                throw std::runtime_error("StreamingHttpRequestParser: poll_character: read returned negative");
            }
            character_max = result.get_bytes_read();
        } else {
            return StreamPollResult<uint8_t>::pending();
        }
    }
    character_head += 1;
    if (character_max == 0)
        return StreamPollResult<uint8_t>::finished();
    uint8_t c = character_buffer[character_head - 1];
    if (character_head == character_max)
        character_head = 0;
    return StreamPollResult<uint8_t>::ready(uint8_t(c)); // LOL
}

}