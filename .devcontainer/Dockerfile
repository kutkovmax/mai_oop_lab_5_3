FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    libgtest-dev \
    cmake \
    gdb \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
