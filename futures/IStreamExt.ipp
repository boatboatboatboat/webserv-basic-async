//
// Created by Djevayo Pattij on 8/21/20.
//

#include "IStreamExt.hpp"

namespace futures {

template <typename T>
template <typename St>
ForEachFuture<St> IStreamExt<T>::for_each(typename ForEachFuture<St>::callback_type function)
{
    auto& x = static_cast<St&>(*this);
    return ForEachFuture<St>(std::move(x), function);
};

template <typename T>
template <typename St>
ForEachFuture<St> IStreamExt<T>::for_each(typename ForEachFuture<St>::callback_type function, void (*eh)(std::exception& e))
{
    auto& x = static_cast<St&>(*this);
    return ForEachFuture<St>(std::move(x), function, eh);
};

}