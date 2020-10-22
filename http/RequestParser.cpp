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
    , last_chunk_size(other.last_chunk_size)
    , chunked(other.chunked)
    , host_set(other.host_set)
    , is_http_1_1(other.is_http_1_1)
    , source(other.source)
    , buffer(move(other.buffer))
    , decoded_body(move(other.decoded_body))
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
    , last_chunk_size(other.last_chunk_size)
    , chunked(other.chunked)
    , host_set(other.host_set)
    , is_http_1_1(other.is_http_1_1)
    , source(new_stream)
    , buffer(move(other.buffer))
    , decoded_body(move(other.decoded_body))
    , builder(move(other.builder))
{
    utils::memcpy(character_buffer, other.character_buffer);
    other.moved = true;
}

// validates field name
static inline void validate_field_name(string_view field_name)
{
    // validate field_name
    if (field_name.empty()) {
        // field name must at least have 1 tchar
        throw RequestParser::MalformedRequest();
    }

    auto field_name_is_invalid = std::any_of(
        field_name.begin(), field_name.end(),
        [](char c) { return !parser_utils::is_tchar(c); });

    if (field_name_is_invalid) {
        throw RequestParser::MalformedRequest();
    }
}

inline static void trim_field_value(string_view& field_value)
{
    // trim OWS from field value
    while (!field_value.empty() && (field_value.front() == ' ' || field_value.front() == '\t')) {
        field_value.remove_prefix(1);
    }
    while (!field_value.empty() && (field_value.back() == ' ' || field_value.back() == '\t')) {
        field_value.remove_suffix(1);
    }
}

inline static void validate_field_value(string_view field_value)
{
    bool last_was_separator = false;

    for (auto c : field_value) {
        if (parser_utils::is_vchar(c)) {
            last_was_separator = false;
        } else if (c == ' ' || c == '\t') {
            last_was_separator = true;
        } else {
            throw RequestParser::MalformedRequest();
        }
    }
    if (last_was_separator) {
        throw RequestParser::MalformedRequest();
    }
}

inline static void validate_accept_charset(string_view field_value)
{
    using namespace parser_utils;
    enum p_state {
        take_charset,
        take_weight
    } state
        = take_charset;
    bool not_first = true;
    // a ; q=1,
    for (;;) {
        if (state == take_charset) {
            if (not_first) {
                not_first = false;
            } else if (parse_optional(field_value, ",")) {
                parse_optional(field_value, " ");
            }
            parse_while(field_value, is_tchar, 1);
            parse_optional(field_value, " ");
            if (field_value.empty()) {
                break;
            }
            if (parse_optional(field_value, ";")) {
                state = take_weight;
            }
        } else if (state == take_weight) {
            parse_optional(field_value, " ");
            parse_expect(field_value, "q");
            parse_expect(field_value, "=");
            parse_expect(field_value, "01");
            if (parse_optional(field_value, ".")) {
                parse_while(field_value, is_digit);
            }
            if (field_value.empty()) {
                break;
            }
            parse_optional(field_value, " ");
            state = take_charset;
        }
    }
}

inline static void validate_accept_language(string_view field_value)
{
    using namespace parser_utils;
    enum p_state {
        take_language,
        take_weight
    } state
        = take_language;
    bool not_first = true;
    // a ; q=1,
    for (;;) {
        if (state == take_language) {
            if (not_first) {
                not_first = false;
            } else if (parse_optional(field_value, ",")) {
                parse_optional(field_value, " ");
            }
            if (parse_optional(field_value, "*")) {
                if (field_value.empty()) {
                    break;
                }
                parse_look_expect(field_value, ";, ");
            } else {
                parse_while(field_value, is_alpha, 1, 8);
                if (parse_optional(field_value, "-")) {
                    if (parse_optional(field_value, "*")) {
                        parse_look_expect(field_value, ";, ");
                    } else {
                        parse_while(field_value, is_alphanumeric, 1, 8);
                    }
                }
            }
            parse_optional(field_value, " ");
            if (field_value.empty()) {
                break;
            }
            if (parse_optional(field_value, ";")) {
                state = take_weight;
            }
        } else if (state == take_weight) {
            parse_optional(field_value, " ");
            parse_expect(field_value, "q");
            parse_expect(field_value, "=");
            parse_expect(field_value, "01");
            if (parse_optional(field_value, ".")) {
                parse_while(field_value, is_digit);
            }
            if (field_value.empty()) {
                break;
            }
            parse_optional(field_value, " ");
            state = take_language;
        }
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
auto RequestParser::inner_poll(Waker&& waker) -> CallState
{
    if (moved)
        return Pending;
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
                case AppendToBody:
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
#ifdef DEBUG_REQUEST_PARSER_STRINGBUFS
                switch (state) {
                    case ReadMethod:
                    case ReadUri:
                    case ReadVersion:
                    case HeaderLine: {
                        DBGPRINT("sibu: " << str_in_buf);
                    } break;
                    default:
                        break;
                }
#endif
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
                            // the header line is empty
                            if (is_http_1_1 && !host_set) {
                                // RFC 7230 5.4.
                                // HTTP/1.1 must have Host header field
                                // otherwise, it is invalid
                                throw MalformedRequest();
                            }
                            if (!method_has_body(current_method)) {
                                // we have a method without a defined body,
                                // we should stop parsing
                                return Completed;
                            }
                            if (content_length.has_value()) {
                                // content-length was set,
                                // so we know the length ahead of time,
                                if (*content_length == 0) {
                                    // switching to body mode with cl=0 will cause
                                    //  the future to never finish.
                                    // instead, return ready now
                                    builder.body_incoming(IncomingBody());
                                    return Completed;
                                }
                                // use the regular body parser
                                state = Body;
                            } else if (chunked) {
                                // we do NOT know the length ahead of time,
                                // but Transfer-Encoding: chunked was set,
                                // so we can use the chunked body parser
                                state = ChunkedBodySize;
                            } else {
                                // No length indication was given,
                                // throw the appropriate status error
                                throw UndeterminedLength();
                            }
                        } else {
                            // the header line is NOT empty
                            // check for ':'
                            auto separator = str_in_buf.find(':');
                            if (separator == string_view::npos) {
                                // there is no ':', so the headerline is invalid
                                throw MalformedRequest();
                            }

                            // field name is everything before the ':'
                            auto field_name = str_in_buf.substr(0, separator);
                            // field value is inside the pseudo-rule `OWS field-value OWS`,
                            // after the ':'
                            auto field_value = str_in_buf.substr(separator + 1);

                            // check whether it's a valid header main-rulewise
                            validate_field_name(field_name);
                            trim_field_value(field_value);
                            validate_field_value(field_value);

                            // check whether it's a valid header header-rulewise
                            {
                                constexpr auto streq = utils::str_eq_case_insensitive;
                                if (streq(field_name, header::CONTENT_LENGTH)) {
                                    // validate Content-Length
                                    auto length = utils::string_to_uint64(field_value);

                                    if (length.has_value()) {
                                        content_length = *length;
                                    } else {
                                        throw MalformedRequest();
                                    }
                                } else if (streq(field_name, header::TRANSFER_ENCODING)) {
                                    // validate Transfer-Encoding
                                    // because the subject doesn't allow zlib,
                                    // we'll consider the transfer encoding invalid
                                    if (field_value != "chunked") {
                                        throw BadTransferEncoding();
                                    }
                                    chunked = true;
                                } else if (streq(field_name, header::HOST)) {
                                    // validate Host header
                                    // TODO: check uri
                                    if (host_set) {
                                        // RFC 7230 5.4.
                                        // if there's more than 1 Host header it's an invalid request
                                        throw MalformedRequest();
                                    }
                                    host_set = true;
                                } else if (streq(field_name, header::USER_AGENT)) {
                                    // validate User-Agent header
                                    auto last_was_space = false;
                                    auto last_was_ver = false;
                                    auto is_token = false;
                                    for (char c : field_value) {
                                        if (c == ' ' || c == '\t') {
                                            if (last_was_ver) {
                                                throw MalformedRequest();
                                            }
                                            last_was_space = true;
                                            last_was_ver = false;
                                        } else if (c == '/') {
                                            if (last_was_space || !is_token) {
                                                throw MalformedRequest();
                                            }
                                            last_was_space = false;
                                            last_was_ver = true;
                                        } else {
                                            is_token = true;
                                            last_was_space = false;
                                            last_was_ver = false;
                                        }
                                    }
                                    if (!is_token || last_was_space || last_was_ver) {
                                        throw MalformedRequest();
                                    }
                                } else if (streq(field_name, header::REFERER)) {
                                    // validate Referer [sic] header
                                    // a Referer = absolute-URI / partial-URI
                                    // a partial-URI can be checked with origin-form mode
                                    auto referer_value = Uri(field_value);
                                    switch (referer_value.get_target_form()) {
                                        case Uri::AbsoluteForm: {
                                        } break;
                                        case Uri::OriginForm: {
                                            // origin form has pqf, so we can deref
                                            // a partial-URI is OriginForm without a fragment
                                            if (referer_value.get_pqf()->get_fragment().has_value()) {
                                                throw MalformedRequest();
                                            }
                                        } break;
                                        default: {
                                            throw MalformedRequest();
                                        } break;
                                    }
                                } else if (streq(field_name, header::ACCEPT_CHARSET)) {
                                    try {
                                        validate_accept_charset(field_value);
                                    } catch (...) {
                                        throw MalformedRequest();
                                    }
                                } else if (streq(field_name, header::ACCEPT_LANGUAGE)) {
                                    try {
                                        validate_accept_language(field_value);
                                    } catch (...) {
                                        throw MalformedRequest();
                                    }
                                }
                            }
                            builder.header(string(field_name), string(field_value));
                        }
                        buffer.clear();
                    } break;
                    case Body: {
                        if (buffer.size() == *content_length) {
                            builder.body_incoming(IncomingBody(move(buffer)));
                            return Completed;
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
                                if (decoded_body.has_value()) {
                                    builder.body_incoming(move(*decoded_body));
                                } else {
                                    builder.body_incoming(IncomingBody());
                                }
                                return Completed;
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
                            // switch to atb
                            state = AppendToBody;
                        }
                    } break;
                    case AppendToBody: {
                        // idk even
                        //                        DBGPRINT("Shiftback ATB re " << poll_result.get() << " " << character_head);
                        shift_back = true;
                        if (!span_reader.has_value()) {
                            span_reader = ioruntime::SpanReader(span(buffer.data(), buffer.size()));
                            if (!decoded_body.has_value()) {
                                decoded_body = IncomingBody();
                            }
                            if ((decoded_body->size() + buffer.size()) > body_limit) {
                                throw BodyExceededLimit();
                            }
                            ricf = ioruntime::RefIoCopyFuture(*span_reader, *decoded_body);
                        }
                        auto icf_res = ricf->poll(Waker(waker));
                        if (icf_res.is_ready()) {
                            if (chunked) {
                                // reset buffer
                                buffer.clear();
                                span_reader = option::nullopt;
                                // switch back to size
                                state = ChunkedBodySize;
                            } else {
                                builder.body_incoming(move(*decoded_body));
                                return Completed;
                            }
                        } else {
                            return Pending;
                        }
                    } break;
                }
            }
            return Running;
        } break;
        case Status::Pending: {
            return Pending;
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
    if (shift_back) {
        shift_back = false;
        character_head -= 1;
        character_buffer_exhausted = false;
    }
    if (character_buffer_exhausted) {
        auto poll_result = source.poll_read(span(character_buffer, sizeof(character_buffer)), move(waker));

        if (poll_result.is_ready()) {
            character_buffer_exhausted = false;
            character_head = 0;
            auto result = poll_result.get();
            if (result.is_error()) {
                throw std::runtime_error("StreamingHttpRequestParser: poll_character: read returned negative");
            }
            character_max = result.get_bytes_read();
        } else {
            return StreamPollResult<uint8_t>::pending();
        }
    }
    if (character_max == 0)
        return StreamPollResult<uint8_t>::finished();
    uint8_t c = character_buffer[character_head];
    character_head += 1;
    if (character_head == character_max) {
        character_buffer_exhausted = true;
    }
    return StreamPollResult<uint8_t>::ready(uint8_t(c)); // LOL
}

auto RequestParser::poll(Waker&& waker) -> PollResult<Request>
{
    auto status = Running;
    while (status == Running) {
        // i think waker copies are cheap so this shouldn't be an issue
        status = inner_poll(Waker(waker));
    }
    if (status == Pending) {
        return PollResult<Request>::pending();
    } else {
        return PollResult<Request>::ready(move(builder).build());
    }
}

}