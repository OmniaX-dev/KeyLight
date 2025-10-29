#!/bin/bash

set -e
mkdir ../dependencies && cd ../dependencies

if [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    # Setup environment
    pacman -Syuu --noconfirm
    pacman -S --noconfirm --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost

    # Build SFML3
    pacman -S --noconfirm --needed git cmake ninja mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libvorbis mingw-w64-ucrt-x86_64-flac mingw-w64-ucrt-x86_64-libogg mingw-w64-ucrt-x86_64-openal mingw-w64-ucrt-x86_64-freetype mingw-w64-ucrt-x86_64-libjpeg-turbo
    git clone --branch 3.0.1 https://github.com/SFML/SFML.git sfml3
    cd sfml3
    cmake -S . -B build-shared -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/ucrt64 \
        -DCMAKE_C_COMPILER=/ucrt64/bin/gcc.exe \
        -DCMAKE_CXX_COMPILER=/ucrt64/bin/g++.exe \
        -DBUILD_SHARED_LIBS=ON
    cmake --build build-shared
    cmake --install build-shared
    cd ..

    # Build TGUI
    git clone https://github.com/texus/TGUI.git
    cd TGUI
    mkdir build && cd build
    cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DTGUI_BACKEND=SFML_GRAPHICS -DCMAKE_INSTALL_PREFIX=/ucrt64
    make -j$(nproc)
    make install
    cd ..

    # Build OmniaFramework
    pacman -S --noconfirm --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_gfx
    git clone https://github.com/OmniaX-dev/OmniaFramework.git
    cd OmniaFramework
    ./build relese
    ./build install
    cd ..
fi
