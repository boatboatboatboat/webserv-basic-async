//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_BOXPTR_HPP
#define WEBSERV_BOXPTR_HPP

#include <algorithm>

namespace boxed {
template <typename T>
class BoxPtr {
    typedef T* t_ptr;

public:
    BoxPtr();
    BoxPtr(BoxPtr&&) = default;
    BoxPtr& operator=(BoxPtr&&) = default;
    ~BoxPtr();
    T& operator*();
    T* operator->();
    T* get();
    template <typename... Args>
    static BoxPtr<T> make(Args&&... args);
    template <typename U>
    BoxPtr<T> operator=(const U& other);

private:
    explicit BoxPtr(t_ptr inner);
    t_ptr inner;
};
} // namespace boxed

#include "BoxPtr.ipp"

#endif // WEBSERV_BOXPTR_HPP
