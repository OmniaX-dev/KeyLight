# ⚠️ !!! THIS PROJECT IS STILL IN EARLY DEVELOPMENT !!! ⚠️

---

### <u>Manual dependency installation process</u>

##### This readme explains how to install dependencies for KeyLight manually, note however that the project includes an automatic script to install dependencies, for more info on that, refer to the main [README](../README.md) file.

---

## <u>Dependencies</u>

KeyLight depends on the following libraries:

- [**OmniaFramework**](https://github.com/OmniaX-dev/OmniaFramework): a modular C++ utility library developed alongside this project

- [**TGUI**](https://tgui.eu/): a modern C++ GUI library built on SFML.

- [**SFML 3**](https://www.sfml-dev.org/): rendering, audio playback, and input handling.

- [**midifile**](https://github.com/craigsapp/midifile): for MIDI parsing and manipulation

These libraries can be built manually from source (instructions down below).

> **<u>NOTE</u>**: the midifile library is included directly as part of the source tree of KeyLight, therefore it doesn't need to be built manually.

---

### <u>Build instructions - Windows</u>

##### Step 1: Install MSYS2

Download and install [**MSYS2**](https://www.msys2.org/), a minimal Unix-like environment for Windows that includes Clang, Make, and package management.

> ⚠️ **Recommended install path:**
> Leave the default location `C:\msys64` unchanged.
> If you install MSYS2 elsewhere, you may need to manually adjust paths in scripts and build commands throughout the build process.

Once installed, launch the **MSYS2 UCRT64 terminal**; this is the environment KeyLight is designed to build in.

##### Step 2: Prepare build environment

Once you have the **MSYS2 UCRT64 terminal** open, run theese two commands to prepare the build environment:

```bash
pacman -Syuu
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-gettext
```

##### Step 3: build SFML3 from source

Still in the **UCRT64** terminal, run the following commands to download, compile and install **SFML3** into your environment:

```bash
pacman -S --needed git cmake ninja mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libvorbis mingw-w64-ucrt-x86_64-flac mingw-w64-ucrt-x86_64-libogg mingw-w64-ucrt-x86_64-openal mingw-w64-ucrt-x86_64-freetype mingw-w64-ucrt-x86_64-libjpeg-turbo
mkdir keylight_dev && cd keylight_dev
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
```

#### Step 4: build TGUI from source

In the **UCRT64** terminal, run the following commands to download, compile and install **TGUI** into your environment:

```bash
git clone https://github.com/texus/TGUI.git
cd TGUI
mkdir build && cd build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DTGUI_BACKEND=SFML_GRAPHICS -DCMAKE_INSTALL_PREFIX=/ucrt64
make -j$(nproc)
make install
cd ..
```

#### Step 5: build OmniaFramework

IN the **UCRT64** terminal, run the following commands to download, compile and install **OmniaFramework** into your environment:

```bash
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_gfx
git clone https://github.com/OmniaX-dev/OmniaFramework.git
cd OmniaFramework
./build relese
./build install
```

#### Step 6: build KeyLight

Finally, still in the UCRT64 terminal, run the following commands to download and compile KeyLight:

```bash
git clone https://github.com/OmniaX-dev/KeyLight.git
cd KeyLight
./build release
```

---

### <u>Build instructions - Linux</u>

This project is officially tested only on the following Linux Distros:

- **Debian based**: Debian, LinuxMint, Ubuntu

- **Arch Based**: ArchLinux, GarudaLinux, Manjaro, EndeavourOS

- **Fedora**

If you are using any other distro, you will have to install the dependencies manually using your package manager.

##### Step 1: Install dependencies

Use your package manager to install git.

- **Arch** Based distros:

```bash
sudo pacman -Syu
sudo pacman -S --needed base-devel git clang gdb cmake \
					    make boost sdl2 sdl2_mixer sdl2_image \
							sdl2_ttf sdl2_gfx ninja gcc libvorbis \
							flac libogg openal freetype2 libjpeg-turbo \
							gettext
```

- **Debian** Based distros:

```bash
sudo apt update
sudo apt install build-essential git dkms linux-headers-generic \
	             clang gdb make cmake libssl-dev libboost-all-dev \
	             libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev \
	             libsdl2-ttf-dev libsdl2-gfx-dev libxcb-randr0-dev libsfml-dev \
		           ninja-build g++ libvorbis-dev libflac-dev libogg-dev libopenal-dev \
		           libfreetype-dev libjpeg-dev gettext
```

- **Fedora**

```bash
sudo dnf install clang gdb make cmake boost-devel SDL2-devel \
				 SDL2_mixer-devel SDL2_image-devel SDL2_ttf-devel \
				 SDL2_gfx-devel libxcb-devel ninja-build gcc-c++ \
				 libvorbis-devel flac-devel libogg-devel \
				 openal-soft-devel freetype-devel libjpeg-turbo-devel \
				 libX11-devel libXrandr-devel libXcursor-devel \
				 libXi-devel systemd-devel ncurses-devel \
				 gettext gettext-devel
```

##### Step 2: build SFML3 from source

```bash
git clone --branch 3.0.1 https://github.com/SFML/SFML.git sfml3
cd sfml3
cmake -S . -B build-shared -G Ninja \
	  -DCMAKE_BUILD_TYPE=Release \
	  -DCMAKE_INSTALL_PREFIX=/usr \
	  -DBUILD_SHARED_LIBS=ON
cmake --build build-shared
sudo cmake --install build-shared
```

##### Step 3: build TGUI from source

```bash
git clone https://github.com/texus/TGUI.git
cd TGUI
mkdir build && cd build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DTGUI_BACKEND=SFML_GRAPHICS -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
sudo make install
cd ../..
```

---

##### Step 4: build OmniaFramework from source

```bash
git clone https://github.com/OmniaX-dev/OmniaFramework.git
cd OmniaFramework
./build release
./build install
cd ..
```

---

##### Step 5: build KeyLight

```bash
git clone https://github.com/OmniaX-Dev/KeyLight
cd KeyLight
./build release
```

---

### <u>Build options</u>

Once **MSYS2** is installed and your environment is set up, you can use the `./build` script in various modes:

##### <u>Debug build</u>

```bash
./build debug
```

Compiles KeyLight with debug symbols and no optimization, ideal for development and troubleshooting.

##### <u>Incremental build (uses last configuration)</u>

```bash
 ./build
```

Rebuilds only the modified source files using **whichever build configuration was last used** (`debug` or `release`).
This is ideal for fast iteration without switching modes.

##### <u>Run after build</u>

```bash
./build run
```

Same as `./build`, but immediately launches the application after building the changes.

##### <u>Windows release packaging</u>

```bash
./build windows_release
```

Creates a full Windows release in `bin/KeyLight_w64/`, including:

- The compiled executable
- All required DLLs
- Assets and resources
- License files

> ⚠️ **Important:**
> 
> **<u>This option is for use on Windows only.</u>** 
> 
> The `build` script assumes MSYS2 is installed at `C:/msys64`.
> If your installation is in a different location, you must manually update the `MSYS_ROOT` variable at the top of `other/build_windows_release.sh`.

##### <u>Linux release packaging</u>

```bash
./build linux_release
```

Creates a Linux release in `bin/KeyLight_linux64/`, including:

- The compiled executable
- Assets and resources
- License files

> ⚠️ **Important:**
> 
> **<u>This option is for use on Linux only</u>**, and nlike the `windows_release` option, the `linux_release` option does not include runtime shared libraries, as bundling them is generally discouraged on Linux.
> This option is intended for **personal use only**, and there is **no guarantee** that the resulting release will work on other Linux systems.
> The preferred practice on Linux is to **build from source on the target system**, ensuring compatibility with its libraries and environment.

##### <u>Install dependencies automatically</u>

```bash
./build dependencies
```

This option is used to install all the needed dependencies, and should only be used **once**, before the first build.

---
