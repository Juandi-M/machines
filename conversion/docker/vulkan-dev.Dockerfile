# Local development tooling; this does not alter the original game's toolchain.
FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    cmake ninja-build g++ ca-certificates \
    gcc-mingw-w64-x86-64-posix g++-mingw-w64-x86-64-posix \
    libsdl2-dev mesa-vulkan-drivers vulkan-validationlayers \
    xvfb xauth glslang-tools \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /work
