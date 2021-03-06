//
// Created by Djevayo Pattij on 8/21/20.
//

#ifndef WEBSERV_FUTURES_ISTREAMEXT_HPP
#define WEBSERV_FUTURES_ISTREAMEXT_HPP

#include "ForEachFuture.hpp"
#include "IStream.hpp"

namespace futures {

template <typename T>
class IStreamExt : public IStream<T> {
public:
    template <typename St>
    ForEachFuture<St> for_each(typename ForEachFuture<St>::callback_type function);
    template <typename St>
    ForEachFuture<St> for_each(typename ForEachFuture<St>::callback_type function, void (*eh)(std::exception& e));
};

}

#include "IStreamExt.ipp"

#endif //WEBSERV_FUTURES_ISTREAMEXT_HPP
