//
// Created by boat on 30-08-20.
//

#ifndef WEBSERV_JSON_INITIALIZER_LIST_HPP
#define WEBSERV_JSON_INITIALIZER_LIST_HPP

#include <cstddef>

namespace lib {
/// initializer_list
template<class RESERVED>
class initializer_list {
public:
    typedef RESERVED value_type;
    typedef const RESERVED &reference;
    typedef const RESERVED &const_reference;
    typedef size_t size_type;
    typedef const RESERVED *iterator;
    typedef const RESERVED *const_iterator;
private:
    iterator M_array;
    size_type M_len;

    // The compiler can call a private constructor.
    constexpr initializer_list(const_iterator _a, size_type _l)
        : M_array(_a), M_len(_l) {}

public:
    constexpr initializer_list() noexcept
        : M_array(nullptr), M_len(0) {}

    // Number of elements.
    [[nodiscard]] constexpr size_type
    size() const noexcept { return M_len; }

    // First element.
    [[nodiscard]] constexpr const_iterator
    begin() const noexcept { return M_array; }

    // One past the last element.
    [[nodiscard]] constexpr const_iterator
    end() const noexcept { return begin() + size(); }
};

/**
 *  @brief  Return an iterator pointing to the first element of
 *          the initializer_list.
 *  @param  _ils  Initializer list.
 */
template<class _Tp>
constexpr const _Tp *
begin(initializer_list<_Tp> _ils) noexcept { return _ils.begin(); }

/**
 *  @brief  Return an iterator pointing to one past the last element
 *          of the initializer_list.
 *  @param  _ils  Initializer list.
 */
template<class _Tp>
constexpr const _Tp *
end(initializer_list<_Tp> _ils) noexcept { return _ils.end(); }
}

#endif //WEBSERV_JSON_INITIALIZER_LIST_HPP
