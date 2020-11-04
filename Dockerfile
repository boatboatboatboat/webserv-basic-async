FROM archlinux:latest AS builder

RUN pacman -Syu base-devel --noconfirm
RUN pacman -Syu llvm --noconfirm
RUN pacman -Syu clang --noconfirm

WORKDIR /webserv_build/

COPY ./args ./args
COPY ./boxed ./boxed
COPY ./cgi ./cgi
COPY ./config ./config
COPY ./constants ./constants
COPY ./fs ./fs
COPY ./func ./func
COPY ./futures ./futures
COPY ./http ./http
COPY ./ioruntime ./ioruntime
COPY ./json ./json
COPY ./modules ./modules
COPY ./mutex ./mutex
COPY ./net ./net
COPY ./option ./option
COPY ./regex ./regex
COPY ./tests ./tests
COPY ./utils ./utils
COPY main.cpp main.cpp
COPY Makefile Makefile

RUN make

###
FROM ubuntu:focal
WORKDIR webserv

COPY --from=builder /webserv_build/webserv /usr/bin/webserv
WORKDIR /tests/
COPY ./tests/ .

CMD ["sh", "test.sh"]
