//
// Created by Djevayo Pattij on 8/21/20.
//

#include "IStreamExt.hpp"

namespace futures {

template <typename T>
template <typename St>
ForEachFuture<St, T> IStreamExt<T>::for_each(void (*function)(T&))
{
    auto& x = static_cast<St&>(*this);
    return ForEachFuture<St, T>(std::move(x), function);
};

template <typename T>
template <typename St>
ForEachFuture<St, T> IStreamExt<T>::for_each(void (*function)(T&), void (*eh)(std::exception& e))
{
    auto& x = static_cast<St&>(*this);
    return ForEachFuture<St, T>(std::move(x), function, eh);
};

}