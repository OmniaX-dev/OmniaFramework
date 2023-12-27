
# OmniaFramework

A collection of C++ classes and tools that I have created over the years


## Build instructions - Windows
**Step 1:**
download MSYS2 from https://www.msys2.org/ and install it

**Step 2:**
run MSYS2, and in the terminal run:
```
pacman -Syuu
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost
```

**Step 3:**
open a UCRT64/MSYS2 command prompt inside the root directory of the project

**Step 4:**
execute this command:
```
./compile
```
## Build instructions - Linux (Arch)
**Step 1:**
open a terminal and run:
```
sudo pacman -Syuu
sudo pacman -S --needed base-devel clang gdb cmake make boost
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
sudo apt install clang gdb make cmake libboost-all-dev
```

**Step 2:**
open a terminal inside the root directory of the project and run this command:
```
./compile
```

#### For other Linux distros, install the dependencies using your package manager.