# ⚠️ !!! THIS PROJECT IS STILL IN EARLY DEVELOPMENT !!! ⚠️

# OmniaFramework

A modular C++ utility library containing a collection of general purpose tools and classses.

### <u>Build instructions - Windows</u>

##### Step 1: Install MSYS2

Download and install [**MSYS2**](https://www.msys2.org/), a minimal Unix-like environment for Windows that includes Clang, Make, and package management.

> ⚠️ **Recommended install path:**
> Leave the default location `C:\msys64` unchanged.
> The build scripts detect the MSYS2 they run in, so another location works too; if detection ever picks the wrong one, pass `-v MSYS2_ROOT=<path>` to `./build` (see [Variables](#variables)).

Once installed, launch the **MSYS2 UCRT64 terminal**; this is the environment OmniaFramework is designed to build in.

##### Step 2: Prepare build environment

Once you have the **MSYS2 UCRT64 terminal** open, install `git` and clone the repository:

```bash
pacman -Syuu
pacman -S --needed git
git clone https://github.com/OmniaX-dev/OmniaFramework.git
cd OmniaFramework
```

Then install the dependencies, either with the build script:

```bash
./build dependencies
```

or by hand:

```bash
pacman -S --needed base-devel git mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-image mingw-w64-ucrt-x86_64-sdl3-ttf mingw-w64-ucrt-x86_64-fluidsynth mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-python
```

> **NOTE**: `./build dependencies` also sets up the tools needed to [rebuild the shaders](#rebuilding-the-shaders). Normal builds don't need them; add `-v SKIP_SHADER_TOOLS=true` to skip that step.

#### Step 3: build OmniaFramework

Still in the **UCRT64** terminal, run the following command to compile and install OmniaFramework:

```bash
./build -i release
```

The libraries are installed into MSYS2's UCRT64 profile (`ucrt64/include`, `ucrt64/bin` and `ucrt64/lib`). See [The build script](#the-build-script) for everything else it can do.

### <u>Build instructions - Linux</u>

##### Step 1: Install dependencies

```bash
git clone https://github.com/OmniaX-dev/OmniaFramework.git
cd OmniaFramework
./build dependencies
```

`./build dependencies` supports Arch-based distros, Ubuntu/Debian/Mint and Fedora. These are the packages it installs, if you prefer doing it by hand:

- **<u>Arch Linux</u>**

```bash
sudo pacman -S --needed base-devel clang gdb cmake make boost sdl3 sdl3_image sdl3_ttf fluidsynth pkgconf libxcb ncurses python
```

- **<u>Ubuntu</u>**

```bash
sudo apt update && sudo apt upgrade
sudo apt install build-essential dkms clang gdb make cmake libssl-dev libboost-all-dev libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev libxcb-randr0-dev libfluidsynth-dev pkg-config libxcb1-dev libncurses-dev python3
```

- **<u>Fedora</u>**

```bash
sudo dnf update
sudo dnf install gcc gcc-c++ make clang gdb cmake clang-tools-extra boost boost-devel SDL3 SDL3_image SDL3_ttf SDL3-devel SDL3_image-devel SDL3_ttf-devel fluidsynth-devel pkg-config libxcb-devel ncurses-devel python3
```

> **NOTE**: For other Linux distros, you need to install the same dependencies through your package manager.

> **NOTE**: On Linux, `./build dependencies` also sets up the tools needed to [rebuild the shaders](#rebuilding-the-shaders) (automatically on Arch-based distros). Normal builds don't need them; add `-v SKIP_SHADER_TOOLS=true` to skip that step.

##### Step 2: Build OmniaFramework

```bash
./build -i release
```

The headers go into `/usr/include` and the libraries into `/usr/lib` (`sudo` is asked for when needed); use `-v INSTALL_DIR=<prefix>` to install somewhere else.

### <u>Build instructions - MacOS</u>

##### Step 1: Install dependencies

```bash
git clone https://github.com/OmniaX-dev/OmniaFramework.git
cd OmniaFramework
./build dependencies
```

`./build dependencies` installs [**Homebrew**](https://brew.sh) if it's missing (and adds it to your `PATH` in `~/.zprofile`), then the dependencies. To do it by hand instead:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

After installing Homebrew, it tells you how to add it to your `PATH`; in general it looks like this:

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

Then install the dependencies (`gdb` is only available on Intel Macs; Apple Silicon uses `lldb`, which comes with the Xcode command line tools):

```bash
brew install git llvm cmake make boost sdl3 sdl3_image sdl3_ttf pkg-config fluidsynth
```

##### Step 2: Build OmniaFramework

```bash
./build -i release
```

The headers go into `/usr/local/include` and the libraries into Homebrew's `lib` folder; use `-v INSTALL_DIR=<prefix>` to install somewhere else.

> **NOTE**: Rebuilding the shaders is **not supported on macOS** (see [Rebuilding the shaders](#rebuilding-the-shaders)). It's never needed for a normal build.

### <u>The build script</u>

Everything is done through `./build`, on every platform:

```
./build [-i] [-c cmake-target] [-t build-target | build-target] [-v VARIABLE=VALUE ...]
```

| Option | Description |
| --- | --- |
| `-i`, `--install` | Also install the libraries and headers, into `INSTALL_DIR` or the platform default. Works with no build target, `release` and `debug`. |
| `-c`, `--cmake-target <target>` | Build only this CMake target (`ostd`, `osnd`, `ogfx`, `ostd_test`, `blur_example`, ...) instead of all of them. |
| `-t`, `--build-target <target>` | What to do (see below). The `-t` can be left out: `./build release` is the same as `./build -t release`. |
| `-v`, `--variables VAR=VALUE ...` | Variable overrides (see below). Every `VAR=VALUE` word after `-v` belongs to it. |
| `-h`, `--help` | Show the usage. |

##### Build targets

| Command | Description |
| --- | --- |
| `./build` | Incremental build: rebuilds only what changed, using **whichever configuration was last used** (`debug` or `release`). Ideal for fast iteration. |
| `./build release` | Clean build of the Release configuration; also increments the build number and creates the release package in `bin/OmniaFramework_<platform>/`. |
| `./build debug` | Same as `release`, with debug symbols and no optimization: ideal for development and troubleshooting. |
| `./build run` | Incremental build, then runs the test application (`ostd_test`). With `-c <target>`, runs that target instead if it's an executable (e.g. `./build -c blur_example run`). |
| `./build dbg` | Like `run`, but under the debugger: `gdb` (log in `bin/gdb-last.log`), or `lldb` on macOS (log in `bin/lldb-last.log`). Prints a full backtrace on a crash. |
| `./build dependencies` | Installs the build dependencies with the system's package manager (see the platform sections above). |
| `./build shaders` | Regenerates the embedded shaders (see [Rebuilding the shaders](#rebuilding-the-shaders)). |

Add `-i` to build and install in one go: `./build -i release`, `./build -i debug`, or `./build -i` (incremental build + install).

All builds run `scripts/fix_tabs.py` first, which normalizes indentation to tabs and strips trailing whitespace in the sources.

##### Variables

Each variable is only read by the build targets that use it; setting one that the chosen target doesn't use just prints a note. Boolean values accept `true`/`false`, `yes`/`no`, `on`/`off` and `1`/`0`. Relative paths are relative to the directory `./build` is run from.

| Variable | Used by | Description |
| --- | --- | --- |
| `INSTALL_DIR=<path>` | `-i` | Install prefix: headers go into `<path>/include`, libraries into `<path>/lib` (on Windows, DLLs into `<path>/bin`). |
| `MSYS2_ROOT=<path>` | `release`, `debug`, `-i` (Windows only) | MSYS2's root folder. Defaults to the MSYS2 the script runs in (normally `C:/msys64`). |
| `SHADERCROSS=<path>` | `shaders` | The `shadercross` executable to use instead of the one in `tools/`. |
| `DXC_LIB_DIR=<path>` | `shaders` | DXC's library folder (with `libdxcompiler` and `libdxil`) to use instead of the one in `tools/`. |
| `NO_DXIL=<bool>` | `shaders` | Leave DXIL (Direct3D 12) out of the shaders. Windows then uses the Vulkan backend. |
| `SKIP_TABS_FIX=<bool>` | all builds | Don't run `scripts/fix_tabs.py` before building. |
| `SKIP_SHADER_TOOLS=<bool>` | `dependencies` | Don't set up the shader tools. |

Examples:

```bash
./build -i release -v INSTALL_DIR=~/.local     # build + install into ~/.local
./build -c ogfx -v SKIP_TABS_FIX=true          # rebuild only libogfx, leave the sources alone
./build dependencies -v SKIP_SHADER_TOOLS=1    # dependencies only, no shader tools
./build shaders -v NO_DXIL=true                # regenerate the shaders without DXIL
```

##### Fish shell completions

`.fish-complete/build.fish` completes the options, build targets, CMake targets and variables of `./build`. Source it from your fish config (for example from a function on `PWD` changes that sources `.fish-complete/*.fish` when the folder exists). It only applies to this checkout's `build` script, so other projects' `build` scripts aren't affected.

### <u>Rebuilding the shaders</u>

OmniaFramework's built-in GPU shaders (`other/shaders/*.frag.hlsl`) are compiled to SPIR-V, MSL and DXIL and embedded in `src/ogfx/resources/EmbeddedShaders.hpp/.cpp`. The generated files are committed, so **normal builds never need to rebuild them**; only changes to the shaders do:

```bash
./build shaders
```

The first run sets up the shader tools in `tools/` (git-ignored) with `scripts/setup-shader-tools.sh`: it builds [SDL_shadercross](https://github.com/libsdl-org/SDL_shadercross) and downloads a pinned, checksum-verified [Microsoft DXC](https://github.com/microsoft/DirectXShaderCompiler/releases) release. Microsoft's release is needed to produce signed DXIL, which Direct3D 12 requires. Nothing is installed system-wide apart from the packages the tools need. The tools' paths are saved in `tools/shader-tools.env`; delete `tools/` to set them up again from scratch.

- **Linux**: supported on Arch-based distros. On other distros, install a C compiler, git, cmake, ninja, curl, SDL3 (development files) and SPIRV-Cross (with `spirv-cross-c-shared`) first.
- **Windows**: supported in the MSYS2 **UCRT64** terminal.
- **macOS**: **not supported**. Microsoft ships no DXC for macOS, and shadercross needs it to read HLSL. Rebuild the shaders on Linux or Windows.

To use your own tools instead, pass `-v SHADERCROSS=<path> DXC_LIB_DIR=<path>`. The CMake target `ogfx_shaders` (`cmake --build bin --target ogfx_shaders`) does the same thing with the tools in `tools/`; its `SHADERCROSS`, `DXC_LIB_DIR` and `NO_DXIL` cache variables override them.
