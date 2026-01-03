# ⚠️ !!! THIS PROJECT IS STILL IN EARLY DEVELOPMENT !!! ⚠️

# OmniaFramework

A modular C++ utility library containing a collection of general purpose tools and classses.

### <u>Build instructions - Windows</u>

##### Step 1: Install MSYS2

Download and install [**MSYS2**](https://www.msys2.org/), a minimal Unix-like environment for Windows that includes Clang, Make, and package management.

> ⚠️ **Recommended install path:**
> Leave the default location `C:\msys64` unchanged.
> If you install MSYS2 elsewhere, you may need to manually adjust paths in scripts and build commands throughout the build process.

Once installed, launch the **MSYS2 UCRT64 terminal**; this is the environment OmniaFramework is designed to build in.

##### Step 2: Prepare build environment

Once you have the **MSYS2 UCRT64 terminal** open, run theese two commands to prepare the build environment and install dependencies:

```bash
pacman -Syuu
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_gfx
```

#### Step 3: build OmniaFramework

Still in the **UCRT64** terminal, run the following commands to download, compile and install OmniaFramework:

```bash
git clone https://github.com/OmniaX-dev/OmniaFramework.git
cd OmniaFramework
./build release
./build install
```

##### Build options

Once **MSYS2** is installed and your environment is set up, you can use the `./build` script to compile OmniaFramework in various modes other than release:

###### <u>Debug build</u>

```bash
./build debug
```

Compiles OmniaFramework with debug symbols and no optimization, ideal for development and troubleshooting.

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

Same as `./build`, but immediately launches `ostd_test.exe` after building the changes.

###### <u>Windows release packaging</u>

```bash
./build install
```

Installs the library MSYS2's UCRT64 profile.

> ⚠️ **Important:**
> The `/build` script assumes MSYS2 is installed at `C:/msys64`.
> If your installation is in a different location, you must manually update the `MSYS_ROOT` variable at the top of `other/create_release.sh` and `other/install_release.sh`.

### 

### <u>Build instructions - Linux</u>

##### Step 1: Install dependencies

- **<u>Arch Linux</u>**

```bash
sudo pacman -S --needed base-devel clang  gdb cmake make boost sdl2 sdl2_mixer sdl2_image sdl2_ttf sdl2_gfx
```

- **<u>Ubuntu</u>**

```bash
sudo apt update && sudo apt upgrade
sudo apt install build-essential dkms linux-headers-$(uname -r) clang gdb make cmake libssl-dev libboost-all-dev libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libxcb-randr0-dev
```

- **<u>Fedora</u>**

```bash
sudo dnf update
sudo dnf install gcc gcc-c++ make clang gdb cmake clang-tools-extra boost boost-devel SDL2 SDL2_image SDL2_mixer SDL2_ttf SDL2_gfx SDL2-devel SDL2_image-devel SDL2_mixer-devel SDL2_ttf-devel SDL2_gfx-devel
```

> **NOTE**: For other Linux distros, you need to install the same dependencies through your package manager.

##### Step 2: Build OmniaFramework

```bash
git clone https://github.com/OmniaX-dev/OmniaFramework.git
cd OmniaFramework
./build release
./build install
```

> **<u>NOTE</u>**: The same build options for the `./build` script apply here, as explained in the **Windows** section.

### <u>Build instructions - MacOS</u>

##### Step 1: Install HomeBrew

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

##### Step 2: Add HomeBrew to PATH

After installing HomeBrew, it should automatically tell you how to add it to path, in general it should loo something. like this:

- **For Apple Silicon**

```bash
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"
```

- **For Intel Mac**

```bash
echo 'eval "$(/usr/local/bin/brew shellenv)"' >> ~/.zprofile
eval "$(/usr/local/bin/brew shellenv)"
```

##### Step 3: Install dependencies

```bash
brew install git llvm gdb cmake make boost sdl2 sdl2_mixer sdl2_image sdl2_ttf sdl2_gfx pkg-config
```

##### Step 4: Build OmniaFramework

```bash
git clone https://github.com/OmniaX-dev/OmniaFramework.git
cd OmniaFramework
./build release
./build install
```

> **<u>NOTE</u>**: The same build options for the `./build` script apply here, as explained in the **Windows** section.
