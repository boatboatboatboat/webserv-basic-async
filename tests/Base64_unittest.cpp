//
// Created by boat on 17-10-20.
//

#include "gtest/gtest.h"
#include "../utils/base64.hpp"

using namespace utils::base64;

namespace {

TEST(Base64, base64_encode_str_hello_world) {
    auto x = encode_string("Hello, world!");

    EXPECT_EQ(x, "SGVsbG8sIHdvcmxkIQ==");
}

TEST(Base64, base64_encode_str_test) {
    auto x = encode_string("this is a test string");

    EXPECT_EQ(x, "dGhpcyBpcyBhIHRlc3Qgc3RyaW5n");
}

TEST(Base64, base64_encode_str_allchar) {
    auto x = encode_string("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ :?!@#$%^&*(){}:\"<>\\. 1234567890");

    EXPECT_EQ(x, "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXogQUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVogOj8hQCMkJV4mKigpe306Ijw+XC4gMTIzNDU2Nzg5MA==");
}

TEST(Base64, base64_encode_str_abc) {
    auto x = encode_string("abc");

    EXPECT_EQ(x, "YWJj");
}

TEST(Base64, base64_encode_str_42) {
    auto x = encode_string("42");

    EXPECT_EQ(x, "NDI=");
}


TEST(Base64, base64_encode_str_h) {
    auto x = encode_string("h");

    EXPECT_EQ(x, "aA==");
}

TEST(Base64, base64_encode_str_empty) {
    auto x = encode_string("");

    EXPECT_EQ(x, "");
}

TEST(Base64, base64_decode_str_hello_world) {
    auto x = decode_string("SGVsbG8sIHdvcmxkIQ==");

    EXPECT_EQ(x, "Hello, world!");
}

TEST(Base64, base64_decode_str_test) {
    auto x = decode_string("dGhpcyBpcyBhIHRlc3Qgc3RyaW5n");

    EXPECT_EQ("this is a test string", x);
}

TEST(Base64, base64_decode_str_allchar) {
    auto x = decode_string("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXogQUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVogOj8hQCMkJV4mKigpe306Ijw+XC4gMTIzNDU2Nzg5MA==");

    EXPECT_EQ("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ :?!@#$%^&*(){}:\"<>\\. 1234567890", x);
}

TEST(Base64, base64_decode_str_abc) {
    auto x = decode_string("YWJj");

    EXPECT_EQ(x, "abc");
}

TEST(Base64, base64_decode_str_42) {
    auto x = decode_string("NDI=");

    EXPECT_EQ(x, "42");
}


TEST(Base64, base64_decode_str_h) {
    auto x = decode_string("aA==");

    EXPECT_EQ(x, "h");
}

TEST(Base64, base64_decode_str_empty) {
    auto x = decode_string("");

    EXPECT_EQ(x, "");
}


}
