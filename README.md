
# OmniaFramework

A collection of C++ classes and tools that I have created over the years


## Build instructions - Windows
**Step 1:**
download MSYS2 from https://www.msys2.org/ and install it	

**Step 2:**
run MSYS2, and in the terminal run:
```
pacman -Syuu
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_gfx mingw-w64-ucrt-x86_64-openssl
```

**Step 3:**
open a UCRT64/MSYS2 command prompt inside the root directory of the project

**Step 4:**
execute theese two commands:
```
./compile
./install
```
## Build instructions - Linux (Arch)
**Step 1:**
open a terminal and run:
```
sudo pacman -Syuu
sudo pacman -S --needed base-devel clang openssl gdb cmake make boost sdl2 sdl2_mixer sdl2_image sdl2_ttf sdl2_gfx
```

**Step 2:**
open a terminal inside the root directory of the project and run theese two commands:
```
./compile
./install
```

## Build instructions - Linux (Ubuntu)
**Step 1:**
open a terminal and run:
```
sudo apt update && sudo apt upgrade
sudo apt install build-essential dkms linux-headers-$(uname -r) clang gdb make cmake libssl-dev libboost-all-dev libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libxcb-randr0-dev
```

**Step 2:**
open a terminal inside the root directory of the project and run theese two commands:
```
./compile
./install
```

## Build instructions - Linux (Fedora)
**Step 1:**
open a terminal and run:
```
sudo dnf update
sudo dnf install gcc gcc-c++ make clang gdb cmake clang-tools-extra boost boost-devel openssl openssl-devel SDL2 SDL2_image SDL2_mixer SDL2_ttf SDL2_gfx SDL2-devel SDL2_image-devel SDL2_mixer-devel SDL2_ttf-devel SDL2_gfx-devel 
```

**Step 2:**
open a terminal inside the root directory of the project and run theese two commands:
```
./compile
./install
```

#### For other Linux distros, install the dependencies using your package manager.	