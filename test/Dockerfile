FROM alpine:latest

RUN apk add --no-cache \
    make \
    gcc \
    git \
    musl-dev \
    bash

RUN apk add --no-cache valgrind

WORKDIR /workspace

COPY ../ .