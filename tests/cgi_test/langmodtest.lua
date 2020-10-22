--
-- Created by IntelliJ IDEA.
-- User: boat
-- Date: 19-10-20
-- Time: 04:24
-- To change this template use File | Settings | File Templates.
--

local body = "";

local function print(...)
    body = body .. table.concat({...}, "\t") .. '\n';
end

local s, e = pcall(function()
    for k, v in pairs(__SCRIPT_ENV) do
        print(k, v, os.getenv(k))
    end
end)

if not s then
    print(e)
end

local ret = "Content-Type: text/plain\r\nContent-Length: "

ret = ret .. #body .. "\r\n\r\n" .. body
io.write(ret)