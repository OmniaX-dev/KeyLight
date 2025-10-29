#!/bin/bash

detect_package_manager() {
  if [[ -f /etc/os-release ]]; then
    source /etc/os-release
    case "$ID" in
      arch|manjaro|endeavouros|garuda)
        echo "pacman"
        ;;
      ubuntu|debian|mint)
        echo "apt"
        ;;
      fedora)
        echo "dnf"
        ;;
      *)
        echo "unsupported"
        ;;
    esac
  else
    echo "unsupported"
  fi
}

install_manual_dependencies_linux() {
    # Build TGUI
    git clone https://github.com/texus/TGUI.git
    cd TGUI
    mkdir build && cd build
    cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DTGUI_BACKEND=SFML_GRAPHICS -DCMAKE_INSTALL_PREFIX=/usr
    make -j$(nproc)
    sudo make install
    cd ../..

    # Build OmniaFramework
	git clone https://github.com/OmniaX-dev/OmniaFramework.git
    cd OmniaFramework
    ./build release
    ./build install
    cd ../..
}

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
    cd ../..

    # Build OmniaFramework
    pacman -S --noconfirm --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_gfx
    git clone https://github.com/OmniaX-dev/OmniaFramework.git
    cd OmniaFramework
    ./build release
    ./build install
    cd ../..
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	pkgmgr=$(detect_package_manager)
	case "$pkgmgr" in
	  pacman) # Arch Based ==================================================================================================
	    sudo pacman -Syu --noconfirm
	    sudo pacman -S --noconfirm --needed base-devel clang openssl gdb cmake make boost sfml sdl2 sdl2_mixer sdl2_image sdl2_ttf sdl2_gfx
	    ;;
	  apt) # Debian Based ==================================================================================================
	    sudo apt update
	    sudo apt install -y build-essential dkms linux-headers-generic \
	      clang gdb make cmake libssl-dev libboost-all-dev \
	      libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev \
	      libsdl2-ttf-dev libsdl2-gfx-dev libxcb-randr0-dev libsfml-dev
	    ;;
	  dnf) # Fedora ==================================================================================================
	    sudo dnf install -y clang gdb make cmake boost-devel SDL2-devel SDL2_mixer-devel SDL2_image-devel SDL2_ttf-devel SDL2_gfx-devel libxcb-devel
	    ;;
	  *)
	    echo "Unsupported distro. Supported distros are: Arch, EndeavourOS, Garuda, Manjaro, Ubuntu, Mint, Debian, Fedora."
	    exit 1
	    ;;
	esac
	install_manual_dependencies_linux
fi
