//
// Created by boat on 9/20/20.
//

#include "../http/Uri.hpp"
#include "../option/optional.hpp"
#include "gtest/gtest.h"

using http::Uri;

namespace {

TEST(UriTests, uri_test_absolute_empty_path)
{
    auto uri = Uri("http://example.com/");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "");
}

TEST(UriTests, uri_test_absolute_test_path)
{
    auto uri = Uri("http://example.com/test");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "test");
}

TEST(UriTests, uri_test_absolute_no_path)
{
    auto uri = Uri("http://example.com");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_FALSE(uri.get_pqf().value().get_path().has_value());
    EXPECT_FALSE(uri.get_pqf().value().get_query().has_value());
    EXPECT_FALSE(uri.get_pqf().value().get_fragment().has_value());
}

TEST(UriTests, uri_test_absolute_path_query)
{
    auto uri = Uri("http://example.com/path?query");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "path");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "query");
    EXPECT_FALSE(uri.get_pqf().value().get_fragment().has_value());
}

TEST(UriTests, uri_test_absolute_path_query_fragment)
{
    auto uri = Uri("http://example.com/path?query#fragment");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "path");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "query");
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_absolute_empty_path_query)
{
    auto uri = Uri("http://example.com/?query");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "query");
    EXPECT_FALSE(uri.get_pqf().value().get_fragment().has_value());
}

TEST(UriTests, uri_test_absolute_no_path_query)
{
    auto uri = Uri("http://example.com?query");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_FALSE(uri.get_pqf().value().get_path().has_value());
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "query");
    EXPECT_FALSE(uri.get_pqf().value().get_fragment().has_value());
}

TEST(UriTests, uri_test_absolute_empty_path_fragment)
{
    auto uri = Uri("http://example.com/#fragment");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "");
    EXPECT_FALSE(uri.get_pqf().value().get_query().has_value());
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_absolute_no_path_fragment)
{
    auto uri = Uri("http://example.com#fragment");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_FALSE(uri.get_pqf().value().get_path().has_value());
    EXPECT_FALSE(uri.get_pqf().value().get_query().has_value());
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_absolute_full)
{
    auto uri = Uri("http://username:password@example.com:1234/this/is/a/path?very=1&big=2&query=3#fragment");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_userinfo().value(), "username:password");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_EQ(uri.get_authority().value().get_port().value(), 1234);
    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "this/is/a/path");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "very=1&big=2&query=3");
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_absolute_full_lmao_this_is_valid)
{
    auto uri = Uri("http://&:!::@example.com:/this/is/a/path???very=1&big=2&query=3#fragment");

    EXPECT_EQ(uri.get_scheme().value().get(), "http");
    EXPECT_EQ(uri.get_authority().value().get_userinfo().value(), "&:!::");
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_FALSE(uri.get_authority().value().get_port().has_value());
    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "this/is/a/path");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "??very=1&big=2&query=3");
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_absolute_escape)
{
    auto uri = Uri("http://example.com/Hello%20World");

    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "Hello%20World");
    EXPECT_EQ(uri.get_pqf().value().get_path_escaped().value(), "Hello World");
}

TEST(UriTests, uri_test_asterisk)
{
    auto uri = Uri("*");

    EXPECT_EQ(uri.get_target_form(), Uri::AsteriskForm);
}

TEST(UriTests, uri_test_authority)
{
    auto uri = Uri("example.com");

    EXPECT_EQ(uri.get_target_form(), Uri::AuthorityForm);
    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
}

TEST(UriTests, uri_test_authority_port)
{
    auto uri = Uri("example.com:42");

    EXPECT_EQ(uri.get_authority().value().get_host(), "example.com");
    EXPECT_EQ(uri.get_authority().value().get_port().value(), 42);
}

TEST(UriTests, uri_test_origin_empty)
{
    auto uri = Uri("/");

    EXPECT_EQ(uri.get_target_form(), Uri::OriginForm);
    EXPECT_EQ(uri.get_pqf()->get_path().value(), "");
}


TEST(UriTests, uri_test_origin_path_query)
{
    auto uri = Uri("/path?query");

    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "path");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "query");
    EXPECT_FALSE(uri.get_pqf().value().get_fragment().has_value());
}

TEST(UriTests, uri_test_origin_path_query_fragment)
{
    auto uri = Uri("/path?query#fragment");

    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "path");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "query");
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_origin_empty_path_query)
{
    auto uri = Uri("/?query");

    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "query");
    EXPECT_FALSE(uri.get_pqf().value().get_fragment().has_value());
}

TEST(UriTests, uri_test_origin_empty_path_fragment)
{
    auto uri = Uri("/#fragment");

    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "");
    EXPECT_FALSE(uri.get_pqf().value().get_query().has_value());
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_origin_full)
{
    auto uri = Uri("/this/is/a/path?very=1&big=2&query=3#fragment");

    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "this/is/a/path");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "very=1&big=2&query=3");
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_origin_full_lmao_this_is_valid)
{
    auto uri = Uri("/this/is/a/path???very=1&big=2&query=3#fragment");

    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "this/is/a/path");
    EXPECT_EQ(uri.get_pqf().value().get_query().value(), "??very=1&big=2&query=3");
    EXPECT_EQ(uri.get_pqf().value().get_fragment().value(), "fragment");
}

TEST(UriTests, uri_test_origin_escape)
{
    auto uri = Uri("/Hello%20World");

    EXPECT_EQ(uri.get_pqf().value().get_path().value(), "Hello%20World");
    EXPECT_EQ(uri.get_pqf().value().get_path_escaped().value(), "Hello World");
}

}