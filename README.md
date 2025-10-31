# ⚠️ !!! THIS PROJECT IS STILL IN EARLY DEVELOPMENT !!! ⚠️

---

# ![icon.png](./other/icon.png) KeyLight

#### A modern alternative to tools like *Embers* and *SeeMusic*

---

**KeyLight** is a free and open-source MIDI piano visualizer designed to create stunning falling-note animations on a virtual keyboard. It offers seamless synchronization between MIDI input and MP3 audio, making it ideal for creating expressive music renderings and performance videos.

---

![screenshot1.png](./other/screenshots/screenshot1.png)

---

### <u>Build instructions - Windows</u>

##### Step 1: Install MSYS2

Download and install [**MSYS2**](https://www.msys2.org/), a minimal Unix-like environment for Windows that includes Clang, Make, and package management.

> ⚠️ **Recommended install path:**
> Leave the default location `C:\msys64` unchanged.
> If you install MSYS2 elsewhere, you may need to manually adjust paths in scripts and build commands throughout the build process.

Once installed, launch the **MSYS2 UCRT64 terminal**; this is the environment KeyLight is designed to build in.

##### Step 2: Prepare build environment

Once you have the **MSYS2 UCRT64 terminal** open, run the following command:

```bash
pacman -Syuu --noconfirm
```

When finished, it's going to ask you to close the terminal and **reopen** it.

##### Step 3: Install dependencies and build KeyLight

Still in the newly opened **UCRT64** terminal, run the following commands to automatically download and install all the dependencies, and build **KeyLight**:

```bash
pacman -S --noconfirm git
git clone https://github.com/OmniaX-Dev/KeyLight
cd KeyLight
./build dependencies
./build windows_release
```

> ⚠️ **Important:**
> 
> The `./build windows_release` command creates a full Windows release in `bin/KeyLight_w64/`, including:
> 
> - The compiled executable
> - All required DLLs
> - Assets and resources
> - License files
> 
> **<u>This option is for use on Windows only.</u>** 
> 
> The `build` script assumes MSYS2 is installed at `C:/msys64`.
> If your installation is in a different location, you must manually update the `MSYS_ROOT` variable at the top of `other/build_windows_release.sh`.
> 
> Refer to the [Manual Build](other/manual_build.md) file for more information on the build process.

---

### <u>Build instructions - Linux</u>

This project is officially tested only on the following Linux Distros:

- **Debian based**: Debian, LinuxMint, Ubuntu

- **Arch Based**: ArchLinux, GarudaLinux, Manjaro, EndeavourOS

- **Fedora**

If you are using any other distro, there is no guarantee that the `build_dependencies.sh` script will work, so you will have to install the dependencies manually using your package manager.

##### Step 1: Install git

Use your package manager to install git.

- **Arch** Based distros:

```bash
sudo pacman -S --needed git
```

- **Debian** Based distros:

```bash
sudo apt install git
```

- **Fedora**

```bash
sudo dnf install git
```

##### Step 2: install dependencies and build KeyLight

Once **git** is installed, clone the **KeyLight** repo and run the build `script` to install the dependencies and build the project.

```bash
git clone https://github.com/OmniaX-Dev/KeyLight
cd KeyLight
./build dependencies
./build linux_release
```

> ⚠️ **Important:**
> 
> The ./build linux_release command creates a Linux release in `bin/KeyLight_linux64/`, including:
> 
> - The compiled executable
> - Assets and resources
> - License files
> 
> **<u>This option is for use on Linux only</u>**, and unlike the `windows_release` option, the `linux_release` option does not include runtime shared libraries, as bundling them is generally discouraged on Linux.
> This option is intended for **personal use only**, and there is **no guarantee** that the resulting release will work on other Linux systems.
> The preferred practice on Linux is to **build from source on the target system**, ensuring compatibility with its libraries and environment.
> 
> Refer to the [Manual Build](other/manual_build.md) file for more information on the build process.

---

### <u>Build options</u>

Once **MSYS2** is installed and your environment is set up, you can use the `./build` script to compile KeyLight in various modes other than release:

###### <u>Debug build</u>

```bash
./build debug
```

Compiles KeyLight with debug symbols and no optimization, ideal for development and troubleshooting.

###### <u>Incremental build (uses last configuration)</u>

```bash
 ./build
```

Rebuilds only the modified source files using **whichever build configuration was last used** (`debug` or `release`).
This is ideal for fast iteration without switching modes.

###### <u>Run after build</u>

```bash
./build run
```

Same as `./build`, but immediately launches the application after building the changes.

###### <u>Windows release packaging</u>

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
> **<u>This option is for use on Windows only.</u>** The `build` script assumes MSYS2 is installed at `C:/msys64`.
> If your installation is in a different location, you must manually update the `MSYS_ROOT` variable at the top of `other/build_windows_release.sh`.

###### <u>Linux release packaging</u>

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

###### <u>Install dependencies automatically</u>

```bash
./build dependencies
```

This option is used to install all the needed dependencies, and should only be used **once**, before the first build.

---
