//
// Created by boat on 9/6/20.
//

#include "../boxed/RcPtr.hpp"
#include "../net/IpAddress.hpp"
#include "../net/SocketAddr.hpp"
#include "gtest/gtest.h"
#include <arpa/inet.h>
#include <iostream>

using net::IpAddress;
using net::Ipv4Address;
using net::Ipv6Address;
using net::SocketAddr;

namespace {

TEST(IpAddressTests, sockaddr_v4_print_1)
{
    sockaddr_in v4ip;
    v4ip.sin_port = htons(1234);
    v4ip.sin_addr.s_addr = inet_addr("1.2.3.4");
    SocketAddr sockadr(v4ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "1.2.3.4:1234");
}

TEST(IpAddressTests, sockaddr_v4_print_2)
{
    sockaddr_in v4ip;
    v4ip.sin_port = htons(4321);
    v4ip.sin_addr.s_addr = inet_addr("0.0.0.0");
    SocketAddr sockadr(v4ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "0.0.0.0:4321");
}

TEST(IpAddressTests, sockaddr_v4_print_3)
{
    sockaddr_in v4ip;
    v4ip.sin_port = htons(80);
    v4ip.sin_addr.s_addr = inet_addr("127.0.0.1");
    SocketAddr sockadr(v4ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "127.0.0.1:80");
}

TEST(IpAddressTests, sockaddr_v6_print_1)
{
    sockaddr_in6 v4ip;
    v4ip.sin6_port = htons(1234);
    v4ip.sin6_addr = IN6ADDR_ANY_INIT;
    SocketAddr sockadr(v4ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[::]:1234");
}

TEST(IpAddressTests, sockaddr_v6_print_2)
{
    sockaddr_in6 v4ip;
    v4ip.sin6_port = htons(1234);
    v4ip.sin6_addr = IN6ADDR_LOOPBACK_INIT;
    SocketAddr sockadr(v4ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[::1]:1234");
}

TEST(IpAddressTests, sockaddr_v6_print_3)
{
    sockaddr_in6 v6ip;
    v6ip.sin6_port = htons(1234);
    inet_pton(AF_INET6, "ff02::1", &v6ip.sin6_addr);
    SocketAddr sockadr(v6ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[ff02::1]:1234");
}

TEST(IpAddressTests, sockaddr_v6_print_4)
{
    sockaddr_in6 v6ip;
    v6ip.sin6_port = htons(1234);
    inet_pton(AF_INET6, "1:2:3:4:5:6:7:8", &v6ip.sin6_addr);
    SocketAddr sockadr(v6ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[1:2:3:4:5:6:7:8]:1234");
}

TEST(IpAddressTests, sockaddr_v6_print_5)
{
    sockaddr_in6 v6ip;
    v6ip.sin6_port = htons(22);
    inet_pton(AF_INET6, "2001:db8::1:0", &v6ip.sin6_addr);
    SocketAddr sockadr(v6ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[2001:db8::1:0]:22");
}

TEST(IpAddressTests, sockaddr_v6_print_6)
{
    sockaddr_in6 v6ip;
    v6ip.sin6_port = htons(5);
    inet_pton(AF_INET6, "2001:db8:0:1:1:1:1:1", &v6ip.sin6_addr);
    SocketAddr sockadr(v6ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[2001:db8:0:1:1:1:1:1]:5");
}

TEST(IpAddressTests, sockaddr_v6_print_7)
{
    sockaddr_in6 v6ip;
    v6ip.sin6_port = htons(42069);
    inet_pton(AF_INET6, "2001:db8::1:0", &v6ip.sin6_addr);
    SocketAddr sockadr(v6ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[2001:db8::1:0]:42069");
}

TEST(IpAddressTests, sockaddr_v6_print_8)
{
    sockaddr_in6 v6ip;
    v6ip.sin6_port = htons(420);
    inet_pton(AF_INET6, "2001:db8:1234::", &v6ip.sin6_addr);
    SocketAddr sockadr(v6ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[2001:db8:1234::]:420");
}

TEST(IpAddressTests, sockaddr_v6_print_9)
{
    sockaddr_in6 v6ip;
    v6ip.sin6_port = htons(420);
    inet_pton(AF_INET6, "1::", &v6ip.sin6_addr);
    SocketAddr sockadr(v6ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[1::]:420");
}

TEST(IpAddressTests, sockaddr_v6_print_10)
{
    sockaddr_in6 v6ip;
    v6ip.sin6_port = htons(420);
    inet_pton(AF_INET6, "1::1", &v6ip.sin6_addr);
    SocketAddr sockadr(v6ip);
    std::stringstream s;
    s << sockadr;
    EXPECT_STREQ(s.str().c_str(), "[1::1]:420");
}

TEST(IpAddressTests, ipaddr_v4_cmp_aton1)
{
    const char* cip = "127.0.0.1";
    Ipv4Address ip(cip);
    in_addr rip {};

    inet_aton(cip, &rip);
    const char* b = strdup(inet_ntoa(in_addr { ip.get_ip_bytes() }));
    EXPECT_STREQ(b, inet_ntoa(rip));
    EXPECT_EQ(ip.get_ip_bytes(), rip.s_addr);
}

TEST(IpAddressTests, ipaddr_v4_cmp_aton2)
{
    const char* cip = "12.34.56.78";
    Ipv4Address ip(cip);
    in_addr rip {};

    inet_aton(cip, &rip);
    const char* b = strdup(inet_ntoa(in_addr { ip.get_ip_bytes() }));
    EXPECT_STREQ(b, inet_ntoa(rip));
    EXPECT_EQ(ip.get_ip_bytes(), rip.s_addr);
}

TEST(IpAddressTests, ipaddr_v4_cmp_aton3)
{
    const char* cip = "0.0.0.0";
    Ipv4Address ip(cip);
    in_addr rip {};

    inet_aton(cip, &rip);
    const char* b = strdup(inet_ntoa(in_addr { ip.get_ip_bytes() }));
    EXPECT_STREQ(b, inet_ntoa(rip));
    EXPECT_EQ(ip.get_ip_bytes(), rip.s_addr);
}

TEST(IpAddressTests, ipaddr_v4_cmp_aton4)
{
    const char* cip = "123.213.42.69";
    Ipv4Address ip(cip);
    in_addr rip {};

    inet_aton(cip, &rip);
    const char* b = strdup(inet_ntoa(in_addr { ip.get_ip_bytes() }));
    EXPECT_STREQ(b, inet_ntoa(rip));
    EXPECT_EQ(ip.get_ip_bytes(), rip.s_addr);
}

TEST(IpAddressTests, ipaddr_v4_cmp_aton5)
{
    const char* cip = "42.69.1";
    Ipv4Address ip(cip);
    in_addr rip {};

    inet_aton(cip, &rip);
    const char* b = strdup(inet_ntoa(in_addr { ip.get_ip_bytes() }));
    EXPECT_STREQ(b, inet_ntoa(rip));
    EXPECT_EQ(ip.get_ip_bytes(), rip.s_addr);
}

TEST(IpAddressTests, ipaddr_v4_cmp_aton6)
{
    const char* cip = "69.42";
    Ipv4Address ip(cip);
    in_addr rip {};

    inet_aton(cip, &rip);
    const char* b = strdup(inet_ntoa(in_addr { ip.get_ip_bytes() }));
    EXPECT_STREQ(b, inet_ntoa(rip));
    EXPECT_EQ(ip.get_ip_bytes(), rip.s_addr);
}

TEST(IpAddressTests, ipaddr_v4_cmp_aton7)
{
    const char* cip = "69";
    Ipv4Address ip(cip);
    in_addr rip {};

    inet_aton(cip, &rip);
    const char* b = strdup(inet_ntoa(in_addr { ip.get_ip_bytes() }));
    EXPECT_STREQ(b, inet_ntoa(rip));
    EXPECT_EQ(ip.get_ip_bytes(), rip.s_addr);
}

TEST(IpAddressTests, ipaddr_v6_cmp_aton1) {
    const char* cip = "::";
    Ipv6Address ip(cip);
    in6_addr oip = ip.get_ip_posix();
    in6_addr rip {};
    char real[128];
    char ours[128];

    inet_pton(AF_INET6, cip, &rip);
    inet_ntop(AF_INET6, &rip, real, sizeof(real));
    inet_ntop(AF_INET6, &oip, ours, sizeof(ours));
    EXPECT_STREQ(real, ours);
}


TEST(IpAddressTests, ipaddr_v6_cmp_aton2) {
    const char* cip = "::1";
    Ipv6Address ip(cip);
    in6_addr oip = ip.get_ip_posix();
    in6_addr rip {};
    char real[128];
    char ours[128];

    inet_pton(AF_INET6, cip, &rip);
    inet_ntop(AF_INET6, &rip, real, sizeof(real));
    inet_ntop(AF_INET6, &oip, ours, sizeof(ours));
    EXPECT_STREQ(real, ours);
}

TEST(IpAddressTests, ipaddr_v6_cmp_aton3) {
    const char* cip = "1::1";
    Ipv6Address ip(cip);
    in6_addr oip = ip.get_ip_posix();
    in6_addr rip {};
    char real[128];
    char ours[128];

    inet_pton(AF_INET6, cip, &rip);
    inet_ntop(AF_INET6, &rip, real, sizeof(real));
    inet_ntop(AF_INET6, &oip, ours, sizeof(ours));
    EXPECT_STREQ(real, ours);
}

TEST(IpAddressTests, ipaddr_str_v6_1) {
    const char* cip = "1::1";
    auto ip = IpAddress::from_str(cip);
    EXPECT_TRUE(ip.is_v6());
    in6_addr oip = ip.get_v6().get_ip_posix();
    in6_addr rip {};
    char real[128];
    char ours[128];

    inet_pton(AF_INET6, cip, &rip);
    inet_ntop(AF_INET6, &rip, real, sizeof(real));
    inet_ntop(AF_INET6, &oip, ours, sizeof(ours));
    EXPECT_STREQ(real, ours);
}

}