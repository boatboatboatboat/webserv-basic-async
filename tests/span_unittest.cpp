//
// Created by boat on 05-10-20.
//

#include "gtest/gtest.h"
#include "../utils/utils.hpp"
#include "../utils/span.hpp"

using namespace utils;

namespace {

TEST(SpanTests, span_iter) {
    uint8_t arr[64];

    for (size_t idx = 0; idx < sizeof(arr); idx += 1) {
        arr[idx] = idx;
    }

    auto s = span(arr, sizeof(arr));
    auto it = s.begin();
    for (size_t idx = 0; idx < sizeof(arr); idx += 1) {
        EXPECT_EQ(*it, arr[idx]);
        ++it;
    }
    EXPECT_EQ(it, s.end());
}

TEST(SpanTests, span_iter_alt) {
    uint16_t arr[128];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = elemcount - idx;
    }

    auto s = span(arr, elemcount);
    auto it = s.begin();
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        EXPECT_EQ(*it, arr[idx]);
        ++it;
    }
    EXPECT_EQ(it, s.end());
}

// FIRST

TEST(SpanTests, span_first) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.first(5);
    EXPECT_EQ(s.size(), 5);
    auto it = s.begin();
    for (size_t idx = 0; idx < 5; idx += 1) {
        EXPECT_EQ(*it, arr[idx]);
        ++it;
    }
    EXPECT_EQ(it, s.end());
}


TEST(SpanTests, span_first_exact_size) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.first(10);
    EXPECT_EQ(
        s.size(),
        10);
    auto it = s.begin();
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        EXPECT_EQ(
            *it,
            arr[idx]);
        ++it;
    }
    EXPECT_EQ(
        it,
        s.end());
}


TEST(SpanTests, span_first_over_size) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.first(11);
    EXPECT_EQ(
        s.size(),
        10);
    auto it = s.begin();
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        EXPECT_EQ(
            *it,
            arr[idx]);
        ++it;
    }
    EXPECT_EQ(
        it,
        s.end());
}

// LAST
TEST(SpanTests, span_last) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.last(5);
    EXPECT_EQ(s.size(), 5);
    auto it = s.begin();
    for (size_t idx = 5; idx < elemcount; idx += 1) {
        EXPECT_EQ(*it, arr[idx]);
        ++it;
    }
    EXPECT_EQ(it, s.end());
}


TEST(SpanTests, span_last_exact_size) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.last(10);
    EXPECT_EQ(
        s.size(),
        10);
    auto it = s.begin();
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        EXPECT_EQ(
            *it,
            arr[idx]);
        ++it;
    }
    EXPECT_EQ(
        it,
        s.end());
}


TEST(SpanTests, span_last_over_size) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.last(11);
    EXPECT_EQ(
        s.size(),
        10);
    auto it = s.begin();
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        EXPECT_EQ(
            *it,
            arr[idx]);
        ++it;
    }
    EXPECT_EQ(
        it,
        s.end());
}

// REMOVE PREFIX

TEST(SpanTests, span_rprefix) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.remove_prefix(3);
    EXPECT_EQ(s.size(), 7);
    auto it = s.begin();
    for (size_t idx = 3; idx < elemcount; idx += 1) {
        EXPECT_EQ(*it, arr[idx]);
        ++it;
    }
    EXPECT_EQ(it, s.end());
}


TEST(SpanTests, span_rprefix_exact) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.remove_prefix(10);
    EXPECT_EQ(
        s.size(),
        0);
    auto it = s.begin();
    EXPECT_EQ(
        it,
        s.end());
}


TEST(SpanTests, span_rprefix_over_size) {
    uint16_t arr[10];
    const size_t elemcount = sizeof(arr) / sizeof(*arr);
    for (size_t idx = 0; idx < elemcount; idx += 1) {
        arr[idx] = idx;
    }

    auto _s = span(arr, elemcount);
    auto s = _s.remove_prefix(11);
    EXPECT_EQ(
        s.size(),
        0);
    auto it = s.begin();
    EXPECT_EQ(
        it,
        s.end());
}


}