//
// Created by boat on 30-08-20.
//

#include "Json.hpp"
#include <cassert>
#include <cmath>

using boxed::RcPtr;

namespace json {
static const int max_depth = 20;

using lib::initializer_list;
using std::map;
using std::move;
using std::string;
using std::vector;

struct NullStruct {
    bool operator==(NullStruct) const { return true; }

    bool operator<(NullStruct) const { return false; }
};

template <Json::Type tag, typename T>
class Value : public JsonValue {
protected:
    explicit Value(const T& value)
        : m_value(value)
    {
    }

    explicit Value(T&& value)
        : m_value(move(value))
    {
    }

    [[nodiscard]] Json::Type type() const override
    {
        return tag;
    }

    bool equals(const JsonValue* other) const override
    {
        return m_value == static_cast<const Value<tag, T>*>(other)->m_value;
    }

    bool less(const JsonValue* other) const override
    {
        return m_value < static_cast<const Value<tag, T>*>(other)->m_value;
    }

    const T m_value;
};

/**
 * Json double type class.
 */
class JsonDouble final : public Value<Json::NUMBER, double> {
    [[nodiscard]] double number_value() const override { return m_value; }
    [[nodiscard]] int int_value() const override { return static_cast<int>(m_value); }
    bool equals(const JsonValue* other) const override { return m_value == other->number_value(); }
    bool less(const JsonValue* other) const override { return m_value < other->number_value(); }

public:
    explicit JsonDouble(double value)
        : Value(value)
    {
    }
};

/**
 * Json integer class.
 */
class JsonInt final : public Value<Json::NUMBER, int> {
    [[nodiscard]] double number_value() const override { return m_value; }
    [[nodiscard]] int int_value() const override { return m_value; }
    bool equals(const JsonValue* other) const override { return m_value == other->number_value(); }
    bool less(const JsonValue* other) const override { return m_value < other->number_value(); }

public:
    explicit JsonInt(int value)
        : Value(value)
    {
    }
};

/**
 * Json boolean class.
 */
class JsonBoolean final : public Value<Json::BOOL, bool> {
    [[nodiscard]] bool bool_value() const override { return m_value; }

public:
    explicit JsonBoolean(bool value)
        : Value(value)
    {
    }
};

/**
 * Json string class.
 */
class JsonString final : public Value<Json::STRING, string> {
    [[nodiscard]] const string& string_value() const override { return m_value; }

public:
    explicit JsonString(const string& value)
        : Value(value)
    {
    }
    explicit JsonString(string&& value)
        : Value(move(value))
    {
    }
};

/**
 * Json array class.
 */
class JsonArray final : public Value<Json::ARRAY, Json::array> {
    [[nodiscard]] const Json::array& array_items() const override { return m_value; }
    const Json& operator[](size_t i) const override;

public:
    explicit JsonArray(const Json::array& value)
        : Value(value)
    {
    }
    explicit JsonArray(Json::array&& value)
        : Value(move(value))
    {
    }
};

/**
 * Json object class. Objects are implemented as a multi map.
 */
class JsonObject final : public Value<Json::OBJECT, Json::object> {
    [[nodiscard]] const Json::object& object_items() const override { return m_value; }
    const Json& operator[](const string& key) const override;

public:
    explicit JsonObject(const Json::object& value)
        : Value(value)
    {
    }
    explicit JsonObject(Json::object&& value)
        : Value(move(value))
    {
    }
};

/**
 * Json null class. Implements the null keyword.
 */
class JsonNull final : public Value<Json::NUL, NullStruct> {
public:
    JsonNull()
        : Value({})
    {
    }
};

/**
 * Implement all default values. This is a smart way to avoid array misses.
 */
struct Statics {
    const RcPtr<JsonValue> null; //RcPtr<JsonNull>::make();
    const RcPtr<JsonValue> t; //icPtr<JsonBoolean>::make(true);
    const RcPtr<JsonValue> f; // = RcPtr<JsonBoolean>::make(false);
    const string empty_string;
    const vector<Json> empty_vector; // = vector<Json>();
    const map<string, Json> empty_map; // = map<string, Json>();

    //    Statics() = default;
};

/**
 * A holder for our statics instance.
 * @return
 */
static const Statics& statics()
{
    static const Statics s {
        RcPtr<JsonNull>::make(),
        RcPtr<JsonBoolean>::make(true),
        RcPtr<JsonBoolean>::make(false),
        string(),
        vector<Json>(),
        map<string, Json>()
    };
    return s;
}

/**
 * Holds the currently parsed Json.
 * @return
 */
static const Json& static_null()
{
    static const Json json_null;
    return json_null;
}

/**
 * All types of JSON constructors.
 */
Json::Json() noexcept
    : m_ptr(RcPtr<JsonNull>::make())
{
}
Json::Json(std::nullptr_t) noexcept
    : m_ptr(statics().null)
{
}
Json::Json(double value)
    : m_ptr(RcPtr<JsonDouble>::make(value))
{
}
Json::Json(int value)
    : m_ptr(RcPtr<JsonInt>::make(value))
{
}
Json::Json(bool value)
    : m_ptr(value ? statics().t : statics().f)
{
}
Json::Json(const string& value)
    : m_ptr(RcPtr<JsonString>::make(value))
{
}
Json::Json(string&& value)
    : m_ptr(RcPtr<JsonString>::make(move(value)))
{
}
Json::Json(const char* value)
    : m_ptr(RcPtr<JsonString>::make(value))
{
}
Json::Json(const Json::array& values)
    : m_ptr(RcPtr<JsonArray>::make(values))
{
}
Json::Json(Json::array&& values)
    : m_ptr(RcPtr<JsonArray>::make(move(values)))
{
}
Json::Json(const Json::object& values)
    : m_ptr(RcPtr<JsonObject>::make(values))
{
}
Json::Json(Json::object&& values)
    : m_ptr(RcPtr<JsonObject>::make(move(values)))
{
}

/**
 * Json methods and overloading to retrieve the values accordingly.
 */
Json::Type Json::type() const { return m_ptr->type(); }
double Json::number_value() const { return m_ptr->number_value(); }
int Json::int_value() const { return m_ptr->int_value(); }
bool Json::bool_value() const { return m_ptr->bool_value(); }
const string& Json::string_value() const { return m_ptr->string_value(); }
const vector<Json>& Json::array_items() const { return m_ptr->array_items(); }
const map<string, Json>& Json::object_items() const { return m_ptr->object_items(); }
const Json& Json::operator[](size_t i) const { return (*m_ptr)[i]; }
const Json& Json::operator[](const string& key) const { return (*m_ptr)[key]; }

/**
 * Convert and retrieve the value
 */
double JsonValue::number_value() const { return 0; }
int JsonValue::int_value() const { return 0; }
bool JsonValue::bool_value() const { return false; }
const string& JsonValue::string_value() const { return statics().empty_string; }
const vector<Json>& JsonValue::array_items() const { return statics().empty_vector; }
const map<string, Json>& JsonValue::object_items() const { return statics().empty_map; }
const Json& JsonValue::operator[](size_t) const { return static_null(); }
const Json& JsonValue::operator[](const string&) const { return static_null(); }

/**
 * Retrieve an string index from an object (property name is the key).
 * @param key
 * @return
 */
const Json& JsonObject::operator[](const string& key) const
{
    auto iter = m_value.find(key);
    return (iter == m_value.end()) ? static_null() : iter->second;
}

/**
 * Get the nth element of a Json array.
 * @param i
 * @return
 */
const Json& JsonArray::operator[](size_t i) const
{
    if (i >= m_value.size())
        return static_null();
    else
        return m_value[i];
}

/**
 * Check if the current Json item equals the other Json item.
 * @param other
 * @return
 */
bool Json::operator==(const Json& other) const
{
    if (m_ptr == other.m_ptr)
        return true;
    if (m_ptr->type() != other.m_ptr->type())
        return false;

    return m_ptr->equals(other.m_ptr.get());
}

/**
 * Check whether a Json element is smaller than another Json element
 * @param other
 * @return
 */
bool Json::operator<(const Json& other) const
{
    if (m_ptr == other.m_ptr)
        return false;
    if (m_ptr->type() != other.m_ptr->type())
        return m_ptr->type() < other.m_ptr->type();

    return m_ptr->less(other.m_ptr.get());
}

/**
 * Escapes a string into a buffer
 * @param c
 * @return
 */
static inline string esc(char c)
{
    char buf[12];
    if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f) {
        snprintf(buf, sizeof buf, "'%c' (%d)", c, c);
    } else {
        snprintf(buf, sizeof buf, "(%d)", c);
    }
    return string(buf);
}

/**
 * Check whether the value x is within a range
 * @param x
 * @param lower
 * @param upper
 * @return
 */
static inline bool in_range(long x, long lower, long upper)
{
    return (x >= lower && x <= upper);
}

namespace {
    /**
 * Json parser structure. Logic for parsing our Json file.
 */
    struct JsonParser final {
        const string& str;
        size_t i;
        string& err;
        bool failed;

        /**
     * Parsing has failed, throws an agnostic error.
     * @param msg
     * @return
     */
        Json fail(string&& msg)
        {
            return error(move(msg), Json());
        }

        /**
     * Error function to throw an error of any type.
     * @tparam T
     * @param msg
     * @param err_ret
     * @return
     */
        template <typename T>
        T error(string&& msg, const T err_ret)
        {
            if (!failed)
                err = std::move(msg);
            failed = true;
            return err_ret;
        }

        /**
     * Trim whitespace.
     */
        void trim()
        {
            while (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t')
                i++;
        }

        /**
     * Get the next character token.
     * @return
     */
        char next_token()
        {
            trim();
            if (failed)
                return static_cast<char>(0);
            if (i == str.size())
                return error("unexpected end of input", static_cast<char>(0));
            return str[i++];
        }

        /**
     * Encode our character to UTF-8 format.
     * @param pt
     * @param out
     */
        void encode_utf8(long pt, string& out)
        {
            if (pt < 0)
                return;

            if (pt < 0x80) {
                out += static_cast<char>(pt);
            } else if (pt < 0x800) {
                out += static_cast<char>((pt >> 6) | 0xC0);
                out += static_cast<char>((pt & 0x3F) | 0x80);
            } else if (pt < 0x10000) {
                out += static_cast<char>((pt >> 12) | 0xE0);
                out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
                out += static_cast<char>((pt & 0x3F) | 0x80);
            } else {
                out += static_cast<char>((pt >> 18) | 0xF0);
                out += static_cast<char>(((pt >> 12) & 0x3F) | 0x80);
                out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
                out += static_cast<char>((pt & 0x3F) | 0x80);
            }
        }

        /**
     * Parse the string
     * @return
     */
        string parse_string()
        {
            string out;
            long last_escaped_codepoint = -1;
            while (true) {
                if (i == str.size())
                    return error("unexpected end of input in string", "");

                char ch = str[i++];

                if (ch == '"') {
                    encode_utf8(last_escaped_codepoint, out);
                    return out;
                }

                if (in_range(ch, 0, 0x1f))
                    return error("unescaped " + esc(ch) + " in string", "");

                // Check for non escaped characters
                if (ch != '\\') {
                    encode_utf8(last_escaped_codepoint, out);
                    last_escaped_codepoint = -1;
                    out += ch;
                    continue;
                }

                // Handle escapes
                if (i == str.size())
                    return error("unexpected end of input in string", "");

                ch = str[i++];

                if (ch == 'u') {
                    // Extract 4-byte escape sequence
                    string esc = str.substr(i, 4);
                    // Explicitly check length of the substring. The following loop
                    // relies on std::string returning the terminating NUL when
                    // accessing str[length]. Checking here reduces brittleness.
                    if (esc.length() < 4) {
                        return error("bad \\u escape: " + esc, "");
                    }
                    for (size_t j = 0; j < 4; j++) {
                        if (!in_range(esc[j], 'a', 'f') && !in_range(esc[j], 'A', 'F')
                            && !in_range(esc[j], '0', '9'))
                            return error("bad \\u escape: " + esc, "");
                    }

                    long codepoint = strtol(esc.data(), nullptr, 16);

                    // JSON specifies that characters outside the BMP shall be encoded as a pair
                    // of 4-hex-digit \u escapes encoding their surrogate pair components. Check
                    // whether we're in the middle of such a beast: the previous codepoint was an
                    // escaped lead (high) surrogate, and this is a trail (low) surrogate.
                    if (in_range(last_escaped_codepoint, 0xD800, 0xDBFF)
                        && in_range(codepoint, 0xDC00, 0xDFFF)) {
                        // Reassemble the two surrogate pairs into one astral-plane character, per
                        // the UTF-16 algorithm.
                        encode_utf8((((last_escaped_codepoint - 0xD800) << 10)
                                        | (codepoint - 0xDC00))
                                + 0x10000,
                            out);
                        last_escaped_codepoint = -1;
                    } else {
                        encode_utf8(last_escaped_codepoint, out);
                        last_escaped_codepoint = codepoint;
                    }

                    i += 4;
                    continue;
                }

                encode_utf8(last_escaped_codepoint, out);
                last_escaped_codepoint = -1;

                // Handle all available escape characters.
                switch (ch) {
                case 'b':
                    out += '\b';
                    break;

                case 'f':
                    out += '\f';
                    break;

                case 'n':
                    out += '\n';
                    break;

                case 'r':
                    out += '\r';
                    break;

                case 't':
                    out += '\t';
                    break;

                case '"':
                case '\\':
                case '/':
                    out += ch;
                    break;

                default:
                    return error("invalid escape character " + esc(ch), "");
                }
            }
        }

        /**
     * Parse a json number
     * @return
     */
        Json parse_number()
        {
            size_t start_pos = i;

            if (str[i] == '-')
                i++;

            // Integer part
            if (str[i] == '0') {
                if (in_range(str[++i], '0', '9'))
                    return fail("leading 0s not permitted in numbers");
            } else if (in_range(str[i], '1', '9')) {
                while (in_range(str[++i], '0', '9'))
                    ;
            } else {
                return fail("invalid " + esc(str[i]) + " in number");
            }

            if (str[i] != '.' && str[i] != 'e' && str[i] != 'E'
                && (i - start_pos) <= static_cast<size_t>(std::numeric_limits<int>::digits10)) {
                return std::atoi(str.c_str() + start_pos);
            }

            // Decimal part
            if (str[i] == '.') {
                if (!in_range(str[++i], '0', '9'))
                    return fail("at least one digit required in fractional part");
                while (in_range(str[i++], '0', '9'))
                    ;
            }

            // Exponent part
            if (str[i] == 'e' || str[i] == 'E') {
                i++;
                if (str[i] == '+' || str[i] == '-')
                    i++;
                if (!in_range(str[i], '0', '9'))
                    return fail("at least one digit required in exponent");
                while (in_range(str[i++], '0', '9'))
                    ;
            }

            return std::strtod(str.c_str() + start_pos, nullptr);
        }

        /**
     * Predict the next part of our token, if they do not match, we throw an error.
     * @param expected
     * @param res
     * @return
     */
        Json expect(const string& expected, Json res)
        {
            assert(i != 0);
            if (str.compare(--i, expected.length(), expected) == 0) {
                i += expected.length();
                return res;
            } else
                return fail("parse error: expected " + expected + ", got " + str.substr(i, expected.length()));
        }

        /**
     * Parse Json with a maximum depth
     * @param depth
     * @return
     */
        Json parse_json(int depth = 20)
        {
            if (depth > max_depth)
                return fail("exceeded maximum nesting depth");

            char ch = next_token();

            if (failed)
                return Json();

            switch (ch) {
            case '-':
            case '0' ... '9':
                i--;
                return parse_number();

            case 't':
                return expect("true", true);

            case 'f':
                return expect("false", false);

            case 'n':
                return expect("null", Json());

            case '"':
                return parse_string();

            case '{': {
                map<string, Json> data;
                ch = next_token();
                if (ch == '}')
                    return data;

                while (true) {
                    if (ch != '"')
                        return fail("expected '\"' in object, got " + esc(ch));

                    string key = parse_string();

                    if (failed)
                        return Json();

                    ch = next_token();

                    if (ch != ':')
                        return fail("expected ':' in object, got " + esc(ch));

                    data[std::move(key)] = parse_json(depth + 1);

                    if (failed)
                        return Json();

                    ch = next_token();

                    if (ch == '}')
                        break;

                    if (ch != ',')
                        return fail("expected ',' in object, got " + esc(ch));

                    ch = next_token();
                }
                return data;
            }

            case '[': {
                vector<Json> data;
                ch = next_token();
                if (ch == ']')
                    return data;

                while (true) {
                    i--;
                    data.push_back(parse_json(depth + 1));
                    if (failed)
                        return Json();

                    ch = next_token();

                    if (ch == ']')
                        break;

                    if (ch != ',')
                        return fail("expected ',' in list, got " + esc(ch));

                    ch = next_token();
                    (void)ch;
                }
                return data;
            }

            default:
                return fail("expected value, got " + esc(ch));
            }
        }
    };
}

/**
 * Parse our Json
 * @param in
 * @param err
 * @return
 */
Json Json::parse(const string& in, string& err)
{
    JsonParser parser { in, 0, err, false };
    Json result = parser.parse_json(0);

    parser.trim();

    if (parser.failed)
        return Json();

    if (parser.i != in.size())
        return parser.fail("unexpected trailing " + esc(in[parser.i]));

    return result;
}
}