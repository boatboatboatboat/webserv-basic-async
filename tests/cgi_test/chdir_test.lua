#!/usr/bin/lua

local f = io.open("./resources/hello.txt", "r")

io.write("Content-Type: text/plain\r\n")

if not f then
    io.write("Content-Length: 4\r\n\r\nfail")
    os.exit(1);
end

local s = "SUCCESS\n" .. f:read("*all");
io.write("Content-Length: " .. s:len() .. "\r\n\r\n");
io.write(s);