//
// Created by boat on 9/6/20.
//

#include "../boxed/RcPtr.hpp"
#include "gtest/gtest.h"

using boxed::RcPtr;

namespace {

TEST(RcPtrTests, UniqueLike) {
    RcPtr<int> x = RcPtr<int>::make(1);

    EXPECT_EQ(x.count(), 1UL);
    RcPtr<int> y = RcPtr(x);
    *y += 1;
    EXPECT_EQ(x.count(), 2UL);
    RcPtr<int> z = std::move(y);
    *z += 1;
    EXPECT_EQ(x.count(), 2UL);
    EXPECT_EQ(*x, 3);
}

}