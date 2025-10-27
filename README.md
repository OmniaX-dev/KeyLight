# KeyLight

A Free and OpenSource MIDI Piano Visualizer

## Build instructions - All
This project requires the OmniaFramework library I wrote, which can be found at https://github.com/OmniaX-dev/OmniaFramework
This library must be correctly installed in clang's search paths (include, lib, bin) before compiling this project.

## Build instructions - Windows
**Step 1:**
download MSYS2 from https://www.msys2.org/ and install it (the project assumes the default install path of c:\msys64, if you install it anywhere else you might have to adjust some scripts/commands accordingly)



**Step 2:**
run MSYS2, and in the terminal run:
```
pacman -Syuu
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_gfx
```

**Step 3:**
open a UCRT64/MSYS2 command prompt

**Step 4:**
in the UCRT64 command prompt, run the following commands to install tgui (dependency of the project)
```
git clone https://github.com/texus/TGUI.git
cd TGUI
mkdir build && cd build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DTGUI_BACKEND=SFML_GRAPHICS -DCMAKE_INSTALL_PREFIX=/ucrt64
make -j$(nproc)
make install
```


**Step 4:**
now, in the UCRT64 command prompt, navigate to the root directory of the project and execute this command to build KeyLight:
```
./compile
```


## Build instructions - Linux (Arch)
**Step 1:**
open a terminal and run:
```
sudo pacman -Syuu
sudo pacman -S --needed base-devel clang openssl gdb cmake make boost sdl2 sdl2_mixer sdl2_image sdl2_ttf sdl2_gfx
```

**Step 2:**
open a terminal inside the root directory of the project and run this command:
```
./compile
```
## Build instructions - Linux (Ubuntu)
**Step 1:**
open a terminal and run:
```
sudo apt update && sudo apt upgrade
sudo apt install build-essential dkms linux-headers-$(uname -r) clang gdb make cmake libssl-dev libboost-all-dev libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libxcb-randr0-dev
```

**Step 2:**
open a terminal inside the root directory of the project and run this command:
```
./compile
```

## Build instructions - Linux (Fedora)
**Step 1:**
open a terminal and run:
```
sudo dnf update
sudo dnf install gcc gcc-c++ make clang gdb cmake clang-tools-extra boost boost-devel openssl openssl-devel SDL2 SDL2_image SDL2_mixer SDL2_ttf SDL2_gfx SDL2-devel SDL2_image-devel SDL2_mixer-devel SDL2_ttf-devel SDL2_gfx-devel
```

#### For other Linux distros, install the dependencies using your package manager.
