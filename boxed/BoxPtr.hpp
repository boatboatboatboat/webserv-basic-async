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
    BoxPtr(BoxPtr&) = delete;
    BoxPtr& operator=(BoxPtr&) = delete;

    ~BoxPtr();
    T& operator*();
    T* operator->();
    T* get();
    void leak();
    const T* get() const;
    template <typename... Args>
    static BoxPtr<T> make(Args&&... args);

    // conversion operators
    template<typename U>
    BoxPtr(BoxPtr<U>&& other);
    template <typename U>
    BoxPtr<T>& operator=(BoxPtr<U>&& other);
    explicit BoxPtr(t_ptr inner);

private:
    t_ptr inner;
};
} // namespace boxed

#include "BoxPtr.ipp"

#endif // WEBSERV_BOXPTR_HPP
