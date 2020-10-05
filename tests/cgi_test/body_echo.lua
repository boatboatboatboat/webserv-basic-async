#!/usr/bin/lua

--
-- Created by IntelliJ IDEA.
-- User: boat
-- Date: 10/5/20
-- Time: 3:49 AM
-- To change this template use File | Settings | File Templates.
--

local len = os.getenv("CONTENT_LENGTH")

if (not len) or not tonumber(len) then
    print("\r\n\r\nwhat the fuck")
    os.exit(0);
end

io.write("Content-Length: " .. len .. "\r\n\r\n");
local t = io.read(tonumber(len));
io.write(t or "FAILED");
