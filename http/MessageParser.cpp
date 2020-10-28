//
// Created by boat on 19-10-20.
//

#include "MessageParser.hpp"
#include "Uri.hpp"

using std::move;
using std::string_view;
using std::string;
using futures::StreamPollResult;

namespace http {

MessageParser::MessageParser(
    ioruntime::CharacterStream& reader,
    size_t buffer_limit,
    size_t body_limit,
    bool has_body)
    : _character_stream(reader)
    , _buffer_limit(buffer_limit)
    , _body_limit(body_limit)
    , _has_body(has_body)
{
}

MessageParser::MessageParser(MessageParser&& other) noexcept
    : _state(other._state)
    , _builder(move(other._builder))
    , _character_stream(other._character_stream)
    , _buffer_limit(other._buffer_limit)
    , _body_limit(other._body_limit)
    , _has_body(other._has_body)
    , _content_length(move(other._content_length))
    , _is_chunk_body(other._is_chunk_body)
    , _host_set(other._host_set)
    , _is_http_1_1(other._host_set)
    , _last_chunk_size(other._last_chunk_size)
    , _buffer(move(other._buffer))
    , _decoded_body(move(other._decoded_body))
    , _span_reader(move(other._span_reader))
    , _ricf(move(other._ricf))
    , _can_eof(other._can_eof)
{
}

MessageParser::MessageParser(MessageParser&& other, ioruntime::CharacterStream& new_stream)
    : _state(other._state)
    , _builder(move(other._builder))
    , _character_stream(new_stream)
    , _buffer_limit(other._buffer_limit)
    , _body_limit(other._body_limit)
    , _has_body(other._has_body)
    , _content_length(move(other._content_length))
    , _is_chunk_body(other._is_chunk_body)
    , _host_set(other._host_set)
    , _is_http_1_1(other._host_set)
    , _last_chunk_size(other._last_chunk_size)
    , _buffer(move(other._buffer))
    , _decoded_body(move(other._decoded_body))
    , _span_reader(move(other._span_reader))
    , _ricf(move(other._ricf))
    , _can_eof(other._can_eof)
{
}

MessageParser::ParserError::ParserError(const char* w)
    : runtime_error(w)
{
}

MessageParser::BodyExceededLimit::BodyExceededLimit()
    : ParserError("Body exceeded limit")
{
}

MessageParser::GenericExceededBuffer::GenericExceededBuffer()
    : ParserError("Buffer exceeded limit")
{
}

MessageParser::MalformedMessage::MalformedMessage()
    : ParserError("Bad HTTP request")
{
}

MessageParser::UnexpectedEof::UnexpectedEof()
    : ParserError("Unexpected end of file")
{
}

MessageParser::UndeterminedLength::UndeterminedLength()
    : ParserError("Undetermined length")
{
}

MessageParser::BadTransferEncoding::BadTransferEncoding()
    : ParserError("Malformed transfer encoding")
{
}

auto MessageParser::poll(Waker&& waker) -> PollResult<IncomingMessage>
{
    auto call_state = Running;
    while (call_state == Running) {
        call_state = inner_poll(Waker(waker));
    }
    if (call_state == Pending) {
        return PollResult<IncomingMessage>::pending();
    } else {
        return PollResult<IncomingMessage>::ready(move(_builder).build());
    }
}

// parser helper functions

inline static void validate_field_name(string_view field_name)
{
    // validate field_name
    if (field_name.empty()) {
        // field name must at least have 1 tchar
        throw MessageParser::MalformedMessage();
    }

    auto field_name_is_invalid = std::any_of(
        field_name.begin(), field_name.end(),
        [](char c) { return !parser_utils::is_tchar(c); });

    if (field_name_is_invalid) {
        throw MessageParser::MalformedMessage();
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
            throw MessageParser::MalformedMessage();
        }
    }
    if (last_was_separator) {
        throw MessageParser::MalformedMessage();
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
auto MessageParser::inner_poll(Waker&& waker) -> MessageParser::CallState
{
    if (_buffer.size() > _buffer_limit) {
        throw GenericExceededBuffer();
    }
    auto poll_result = _character_stream.poll_next(Waker(waker));
    switch (poll_result.get_status()) {
        case StreamPollResult<uint8_t>::Status::Pending: {
            // there are no characters ready - return pending
            return Pending;
        } break;
        case StreamPollResult<uint8_t>::Status::Ready: {
            auto character = poll_result.get();
            switch (_state) {
                case HeaderLine: {
                    if (character == '\n' && !_buffer.empty() && _buffer.back() == '\r') {
                        // if it ends on \r\n
                        // remove the \r that's in the buffer
                        _buffer.pop_back();
                    } else {
                        // otherwise, add the character to the buffer and 'continue'
                        _buffer.push_back(character);
                        return Running;
                    }
                    auto string_in_buffer = string_view(reinterpret_cast<const char*>(_buffer.data()), _buffer.size());
                    if (string_in_buffer.empty()) {
                        // The header line is empty
                        // ; we are at the header terminator
                        if (_is_http_1_1 && !_host_set) {
                            // RFC 7230 5.4.
                            // HTTP/1.1 must have Host header field
                            // otherwise, it is invalid
                            throw MalformedMessage();
                        }
                        if (!_has_body) {
                            // this message does not have to parse a body,
                            // we should stop now
                            return Completed;
                        }
                        if (_content_length.has_value()) {
                            // content-length was set, so we know the length AOT
                            if (*_content_length == 0) {
                                _builder.body(IncomingBody());
                                return Completed;
                            }
                            _state = Body;
                        } else if (_is_chunk_body) {
                            // we do NOT know the length ahead of time,
                            // but Transfer-Encoding: chunked was set,
                            // so we can use the chunked body parser
                            _state = ChunkedBodySize;
                        } else {
                            // No length indication was given,
                            // throw the appropriate status error
                            _can_eof = true;
                            _state = Body;
                        }
                        return Running;
                    } else {
                        // the header line is NOT empty - check for ':'

                        auto separator = string_in_buffer.find(':');
                        if (separator == string_view::npos) {
                            // there is no ':', so the headerline is invalid
                            throw MalformedMessage();
                        }

                        // field name is everything before the ':'
                        auto field_name = string_in_buffer.substr(0, separator);
                        // field value is inside the pseudo-rule `OWS field-value OWS`,
                        // after the ':'
                        auto field_value = string_in_buffer.substr(separator + 1);

                        // check whether it's a valid header main-rulewise
                        validate_field_name(field_name);
                        trim_field_value(field_value);
                        validate_field_value(field_value);

                        // check whether it's a valid header rfc-wise
                        constexpr auto str_eq = utils::str_eq_case_insensitive;
                        if (str_eq(field_name, header::CONTENT_LENGTH)) {
                            // validate Content-Length
                            auto length = utils::string_to_uint64(field_value);

                            if (length.has_value()) {
                                _content_length = *length;
                            } else {
                                throw MalformedMessage();
                            }
                        } else if (str_eq(field_name, header::TRANSFER_ENCODING)) {
                            // validate Transfer-Encoding
                            // because the subject doesn't allow zlib,
                            // we'll consider the transfer encoding invalid
                            if (field_value != "chunked") {
                                throw BadTransferEncoding();
                            }
                            _is_chunk_body = true;
                        } else if (str_eq(field_name, header::HOST)) {
                            Uri u(field_value);
                            if (u.get_target_form() != Uri::AuthorityForm) {
                                // host is not in authority form
                                throw MalformedMessage();
                            }
                            if (_host_set) {
                                // RFC 7230 5.4.
                                // if there's more than 1 Host header it's an invalid request
                                throw MalformedMessage();
                            }
                            _host_set = true;
                        } else if (str_eq(field_name, header::USER_AGENT)) {
                            // validate User-Agent header
                            auto last_was_space = false;
                            auto last_was_ver = false;
                            auto is_token = false;
                            for (char c : field_value) {
                                if (c == ' ' || c == '\t') {
                                    if (last_was_ver) {
                                        throw MalformedMessage();
                                    }
                                    last_was_space = true;
                                    last_was_ver = false;
                                } else if (c == '/') {
                                    if (last_was_space || !is_token) {
                                        throw MalformedMessage();
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
                                throw MalformedMessage();
                            }
                        } else if (str_eq(field_name, header::REFERER)) {
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
                                        throw MalformedMessage();
                                    }
                                } break;
                                default: {
                                    throw MalformedMessage();
                                } break;
                            }
                        } else if (str_eq(field_name, header::ACCEPT_CHARSET)) {
                            try {
                                validate_accept_charset(field_value);
                            } catch (...) {
                                throw MalformedMessage();
                            }
                        } else if (str_eq(field_name, header::ACCEPT_LANGUAGE)) {
                            try {
                                validate_accept_language(field_value);
                            } catch (...) {
                                throw MalformedMessage();
                            }
                        }
                        _builder.header(string(field_name), string(field_value));
                    }
                    _buffer.clear();
                    return Running;
                } break;
                case Body: {
                    _buffer.push_back(character);
                    if (_buffer.size() > _body_limit) {
                        throw BodyExceededLimit();
                    }
                    if (!_can_eof && _buffer.size() == *_content_length) {
                        _builder.body(IncomingBody(move(_buffer)));
                        return Completed;
                    }
                    return Running;
                } break;
                case ChunkedBodySize: {
                    if (character == '\n' && !_buffer.empty() && _buffer.back() == '\r') {
                        // if it ends on \r\n
                        // remove the \r that's in the buffer
                        _buffer.pop_back();
                    } else {
                        // otherwise, add the character to the buffer and 'continue'
                        _buffer.push_back(character);
                        return Running;
                    }
                    // basically, a size can be something like
                    // FF;12=34;56=78
                    // however we are allowed to ignore the extensions,
                    // so we're just going to ignore everything after the first ';'.
                    // however, we still have to check if the characters are valid
                    // because we need to send a 400 if that isn't the case

                    auto string_in_buffer = string_view(reinterpret_cast<const char*>(_buffer.data()), _buffer.size());
                    auto first_separator = string_in_buffer.find(';');
                    if (first_separator == string_view::npos) {
                        first_separator = string_in_buffer.length();
                    } else {
                        for (auto& c : string_in_buffer.substr(first_separator, string_in_buffer.length())) {
                            if (!parser_utils::is_tchar(c) && c != ';') {
                                throw MalformedMessage();
                            }
                        }
                    }

                    auto decoded_size = utils::hexstring_to_uint64(string_in_buffer.substr(0, first_separator));
                    if (decoded_size.has_value()) {
                        _last_chunk_size = decoded_size.value();
                        if (decoded_size.value() == 0) {
                            if (_decoded_body.has_value()) {
                                _builder.body(move(*_decoded_body));
                            } else {
                                _builder.body(IncomingBody());
                            }
                            return Completed;
                        }
                        // check if this would exceed the limit
                        if (_decoded_body.has_value()) {
                            if ((_decoded_body->size() + *decoded_size) > _body_limit) {
                                throw BodyExceededLimit();
                            }
                        } else if (*decoded_size > _body_limit) {
                            throw BodyExceededLimit();
                        }
                        _buffer.clear();
                        _state = ChunkedBodyData;
                    } else {
                        throw MalformedMessage();
                    }
                    return Running;
                } break;
                case ChunkedBodyData: {
                    _buffer.push_back(character);
                    if (_buffer.size() == _last_chunk_size + 2) {
                        if (_buffer.back() != '\n') {
                            throw MalformedMessage();
                        }
                        _buffer.pop_back();
                        if (_buffer.back() != '\r') {
                            throw MalformedMessage();
                        }
                        _buffer.pop_back();
                        _state = AppendToBody;
                    }
                    return Running;
                } break;
                case AppendToBody: {
                    _character_stream.shift_back(character);
                    if (!_span_reader.has_value()) {
                        _span_reader = ioruntime::SpanReader(span(_buffer.data(), _buffer.size()));
                        if (!_decoded_body.has_value()) {
                            _decoded_body = IncomingBody();
                        }
                        if ((_decoded_body->size() + _buffer.size()) > _body_limit) {
                            throw BodyExceededLimit();
                        }
                        _ricf = ioruntime::RefIoCopyFuture(*_span_reader, *_decoded_body);
                    }
                    auto icf_res = _ricf->poll(Waker(waker));
                    if (icf_res.is_ready()) {
                        if (_is_chunk_body) {
                            _buffer.clear();
                            _span_reader.reset();
                            _state = ChunkedBodySize;
                            return Running;
                        } else {
                            _builder.body(move(*_decoded_body));
                            return Completed;
                        }
                    } else {
                        return Pending;
                    }
                } break;
            }
            return Running;
        } break;
        case StreamPollResult<uint8_t>::Status::Finished: {
            // All EOFs are unexpected - TCE/CL indicates the end.
            if (_can_eof) {
                _builder.body(IncomingBody(move(_buffer)));
                return Completed;
            } else {
                throw UnexpectedEof();
            }
        } break;
    }
    return Pending;
}
}
