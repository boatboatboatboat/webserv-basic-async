//
// Created by boat on 9/20/20.
//

#include "../option/optional.hpp"
#include "gtest/gtest.h"

using option::optional;

namespace {

TEST(OptionalTests, optional_default_ctor_eq_false)
{
    optional<int> x;
    EXPECT_FALSE(x);
}

TEST(OptionalTests, optional_cctor_nullopt_eq_false)
{
    optional<int> x = option::nullopt;
    EXPECT_FALSE(x);
}

TEST(OptionalTests, optional_cctor_nullopt_has_value)
{
    optional<int> x = option::nullopt;
    EXPECT_FALSE(x.has_value());
}

TEST(OptionalTests, optional_default_ctor_has_value)
{
    optional<int> x;
    EXPECT_FALSE(x.has_value());
}

TEST(OptionalTests, optional_value_or)
{
    optional<int> x;

    EXPECT_EQ(x.value_or(4), 4);
}

TEST(OptionalTests, optional_value_none_error)
{
    EXPECT_THROW({
        optional<int> x;

        (void)x.value();
    },
        std::runtime_error);
}

TEST(OptionalTests, optional_value_some)
{
    EXPECT_NO_THROW({
        optional<int> x = 1;

        EXPECT_EQ(x.value(), 1);
    });
}

TEST(OptionalTests, optional_deref)
{
    optional<int> x = 1;

    EXPECT_EQ(*x, 1);
}

TEST(OptionalTests, optional_deref_err)
{
    EXPECT_THROW({
        optional<int> x;
        auto y = *x;
        (void)y;
    },
        std::runtime_error);
}

TEST(OptionalTests, optional_deref_move)
{
    auto x = optional<std::string> { "hello" };
    auto y = *std::move(x);

    EXPECT_FALSE(x);
    EXPECT_STREQ(y.c_str(), "hello");
}

}