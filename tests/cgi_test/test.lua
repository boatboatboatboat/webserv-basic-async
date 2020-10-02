#!/usr/bin/lua

print("content-type:text/html\r\n\r\n")
print(os.getenv("REQUEST_URI"))
