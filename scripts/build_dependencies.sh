#!/bin/bash
# build_dependencies.sh
#
# Installs everything needed to build OmniaFramework with the system's package manager,
# then sets up the shader tools (scripts/setup-shader-tools.sh) on Linux and Windows.
#
# Usage: scripts/build_dependencies.sh [--skip-shader-tools]
# Normally run by ./build dependencies (SKIP_SHADER_TOOLS=true passes --skip-shader-tools).

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKIP_SHADER_TOOLS=0
for arg in "$@"; do
	case "$arg" in
		--skip-shader-tools) SKIP_SHADER_TOOLS=1 ;;
		*) echo "unknown argument '$arg'" >&2; exit 1 ;;
	esac
done

# Only needed to regenerate the embedded shaders (./build shaders), not for normal builds.
setup_shader_tools() {
	if [[ $SKIP_SHADER_TOOLS -eq 1 ]]; then
		echo "Skipping the shader tools setup"
	else
		bash "$SCRIPT_DIR/setup-shader-tools.sh"
	fi
}

detect_package_manager() {
	if [[ -f /etc/os-release ]]; then
		source /etc/os-release
		case "$ID" in
			arch|manjaro|endeavouros|garuda|cachyos)
				echo "pacman"
				;;
			ubuntu|debian|linuxmint)
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

# macOS: make sure Homebrew is installed and usable from this script (and future shells)
ensure_brew() {
	# Homebrew's install location depends on the CPU architecture
	local brew_prefix
	if [[ "$(uname -m)" == "arm64" ]]; then
		brew_prefix="/opt/homebrew"
	else
		brew_prefix="/usr/local"
	fi
	local brew_bin="$brew_prefix/bin/brew"

	# Already on PATH: nothing to do
	if command -v brew >/dev/null 2>&1; then
		return 0
	fi

	# Not installed at all: run the official installer
	if [[ ! -x "$brew_bin" ]]; then
		echo "Homebrew not found, installing..."
		/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
	fi

	# Installed but not on PATH: load it into this script's environment
	eval "$("$brew_bin" shellenv)"

	# Persist for future shells, only once
	local line="eval \"\$($brew_bin shellenv)\""
	if ! grep -qsF "$line" "$HOME/.zprofile"; then
		echo "$line" >> "$HOME/.zprofile"
		echo "Added Homebrew to ~/.zprofile"
	fi
}

set -e

case "$(uname -s)" in
	MINGW64_NT*) # Windows (MSYS2) ======================================================================================
		# Setup environment
		pacman -Syuu --noconfirm
		pacman -S --noconfirm --needed base-devel git mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make \
								   	   mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-image mingw-w64-ucrt-x86_64-sdl3-ttf \
								       mingw-w64-ucrt-x86_64-fluidsynth mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-python
		setup_shader_tools
		;;
	Linux) # Linux ======================================================================================================
		pkgmgr=$(detect_package_manager)
		case "$pkgmgr" in
			pacman) # Arch Based ========================================================================================
				sudo pacman -Syu --noconfirm
				sudo pacman -S --noconfirm --needed base-devel clang gdb cmake make boost sdl3 sdl3_image sdl3_ttf fluidsynth pkgconf libxcb ncurses python
				;;
			apt) # Debian Based =========================================================================================
				sudo apt update -y && sudo apt upgrade -y
				sudo apt install -y build-essential dkms linux-headers-$(uname -r) clang gdb make cmake libssl-dev libboost-all-dev libsdl3-dev libsdl3-image-dev \
									libsdl3-ttf-dev libxcb-randr0-dev libfluidsynth-dev pkg-config libxcb1-dev libncurses-dev python3
				;;
			dnf) # Fedora ===============================================================================================
				sudo dnf update -y
				sudo dnf install -y gcc gcc-c++ make clang gdb cmake clang-tools-extra boost boost-devel SDL3 SDL3_image SDL3_ttf SDL3-devel SDL3_image-devel \
									SDL3_ttf-devel fluidsynth-devel pkg-config libxcb-devel ncurses-devel python3
				;;
			*)
				echo "Unsupported distro. Supported distros are: Arch, EndeavourOS, Garuda, Manjaro, Ubuntu, Mint, Debian, Fedora."
				exit 1
				;;
		esac
		setup_shader_tools
		;;
	Darwin) # macOS =====================================================================================================
		ensure_brew
		brew install git llvm cmake make boost sdl3 sdl3_image sdl3_ttf pkg-config fluidsynth
		# GDB doesn't support Apple Silicon (use lldb there); install it on Intel Macs only
		if [[ "$(uname -m)" == "x86_64" ]]; then
			brew install gdb
		fi
		;;
	*)
		echo "Unsupported OS: $(uname -s)"
		exit 1
		;;
esac
