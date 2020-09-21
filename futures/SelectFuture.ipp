//
// Created by Djevayo Pattij on 8/26/20.
//

#include "SelectFuture.hpp"
#include "Waker.hpp"

namespace futures {

template <typename Ret, typename FutX, typename FutY>
PollResult<Ret> SelectFuture<Ret, FutX, FutY>::poll(Waker&& waker)
{
    auto res = x.poll(Waker(waker));
    if (res.is_ready()) {
        return res;
    } else {
        return y.poll(std::move(waker));
    }
}
template <typename Ret, typename FutX, typename FutY>
SelectFuture<Ret, FutX, FutY>::SelectFuture(FutX&& x, FutY&& y)
    : x(std::move(x))
    , y(std::move(y))
{
}

template <typename Ret, typename FutX, typename FutY>
static SelectFuture<Ret, FutX, FutY> select(FutX&& x, FutY&& y)
{
    return SelectFuture<Ret, FutX, FutY>(std::forward<FutX>(x), std::forward<FutY>(y));
}

}