//
// Created by boat on 30-08-20.
//

#ifndef WEBSERV_JSON_JSON_HPP
#define WEBSERV_JSON_JSON_HPP

#include "../boxed/RcPtr.hpp"
#include "initializer_list.hpp"
#include <map>
#include <string>
#include <vector>

using boxed::RcPtr;

namespace json {
class JsonValue;

/**
 * Json class.
 */
class Json final {
public:
    /**
     * We have 6 different available types.
     */
    enum Type {
        NUL,
        NUMBER,
        BOOL,
        STRING,
        ARRAY,
        OBJECT
    };

    /**
     * We have a map and vector for arrays and objects.
     */
    typedef std::vector<Json> array;
    typedef std::map<std::string, Json> object;

    Json() noexcept;
    Json(std::nullptr_t) noexcept;
    Json(double value);
    Json(int value);
    Json(bool value);
    Json(const std::string& value);
    Json(std::string&& value);
    Json(const char* value);
    Json(const array& values);
    Json(array&& values);
    Json(const object& values);
    Json(object&& values);

    template <class T, class = decltype(&T::to_json)>
    explicit Json(const T& t)
        : Json(t.to_json())
    {
    }

    template <class M, typename std::enable_if<std::is_constructible<std::string, decltype(std::declval<M>().begin()->first)>::value && std::is_constructible<Json, decltype(std::declval<M>().begin()->second)>::value, int>::type = 0>
    explicit Json(const M& m)
        : Json(object(m.begin(), m.end()))
    {
    }

    template <class V, typename std::enable_if<std::is_constructible<Json, decltype(*std::declval<V>().begin())>::value, int>::type = 0>
    explicit Json(const V& v)
        : Json(array(v.begin(), v.end()))
    {
    }

    Json(void*);

    // Accessors
    [[nodiscard]] Type type() const;
    [[nodiscard]] bool is_null() const { return type() == NUL; }
    [[nodiscard]] bool is_number() const { return type() == NUMBER; }
    [[nodiscard]] bool is_bool() const { return type() == BOOL; }
    [[nodiscard]] bool is_string() const { return type() == STRING; }
    [[nodiscard]] bool is_array() const { return type() == ARRAY; }
    [[nodiscard]] bool is_object() const { return type() == OBJECT; }

    // Return the enclosed value if this is a number, 0 otherwise. Note that integer arithmetic works regardless of
    // number type.
    [[nodiscard]] double number_value() const;
    [[nodiscard]] int int_value() const;
    [[nodiscard]] bool bool_value() const;
    [[nodiscard]] const std::string& string_value() const;
    [[nodiscard]] const array& array_items() const;
    [[nodiscard]] const object& object_items() const;

    const Json& operator[](size_t i) const;
    const Json& operator[](const std::string& key) const;

    // Parse. If parse fails, return Json() and assign an error message to err.
    static Json parse(const std::string& in, std::string& err);
    static Json parse(const char* in, std::string& err)
    {
        if (in) {
            return parse(std::string(in), err);
        } else {
            err = "null input";
            return nullptr;
        }
    }

    bool operator==(const Json& rhs) const;
    bool operator<(const Json& rhs) const;
    bool operator!=(const Json& rhs) const { return !(*this == rhs); }
    bool operator<=(const Json& rhs) const { return !(rhs < *this); }
    bool operator>(const Json& rhs) const { return (rhs < *this); }
    bool operator>=(const Json& rhs) const { return !(*this < rhs); }

    typedef lib::initializer_list<std::pair<std::string, Type>> shape;

private:
    RcPtr<JsonValue> m_ptr;
};

class JsonValue {
protected:
    friend class Json;
    friend class JsonInt;
    friend class JsonDouble;

    virtual Json::Type type() const = 0;
    virtual bool equals(const JsonValue* other) const = 0;
    virtual bool less(const JsonValue* other) const = 0;

    [[nodiscard]] virtual double number_value() const;
    [[nodiscard]] virtual int int_value() const;
    [[nodiscard]] virtual bool bool_value() const;
    [[nodiscard]] virtual const std::string& string_value() const;
    [[nodiscard]] virtual const Json::array& array_items() const;
    [[nodiscard]] virtual const Json::object& object_items() const;

    virtual const Json& operator[](size_t i) const;
    virtual const Json& operator[](const std::string& key) const;

public:
    // FIXME: how even
    virtual ~JsonValue() = default;
};
}

#endif //WEBSERV_JSON_JSON_HPP
