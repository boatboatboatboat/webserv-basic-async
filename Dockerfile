FROM archlinux:latest AS builder

RUN pacman -Syu base-devel --noconfirm
RUN pacman -Syu llvm --noconfirm
RUN pacman -Syu clang --noconfirm

WORKDIR /webserv_build/
COPY . .
RUN make -j

FROM ubuntu:focal
WORKDIR webserv

COPY --from=builder /webserv_build/webserv /usr/bin/webserv
WORKDIR /tests/
COPY ./tests/ .

CMD ["sh", "test.sh"]
