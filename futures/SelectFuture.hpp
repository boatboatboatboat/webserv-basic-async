//
// Created by Djevayo Pattij on 8/26/20.
//

#ifndef WEBSERV_FUTURES_SELECTFUTURE_HPP
#define WEBSERV_FUTURES_SELECTFUTURE_HPP

#include "IFuture.hpp"

namespace futures {

template <typename Ret, typename FutX, typename FutY>
class SelectFuture : public IFuture<Ret> {
public:
    SelectFuture(FutX&& x, FutY&& y);
    PollResult<Ret> poll(Waker&& waker) override;

private:
    FutX x;
    FutY y;
};

template <typename Ret, typename FutX, typename FutY>
static SelectFuture<Ret, FutX, FutY> select(FutX&& x, FutY&& y);

}

#include "SelectFuture.ipp"

#endif //WEBSERV_FUTURES_SELECTFUTURE_HPP
