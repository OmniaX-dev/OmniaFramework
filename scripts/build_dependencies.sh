#!/bin/bash

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
								       mingw-w64-ucrt-x86_64-fluidsynth mingw-w64-ucrt-x86_64-pkgconf
		./setup-shader-tools.sh
		;;
	Linux) # Linux ======================================================================================================
		pkgmgr=$(detect_package_manager)
		case "$pkgmgr" in
			pacman) # Arch Based ========================================================================================
				sudo pacman -Syu --noconfirm
				sudo pacman -S --noconfirm --needed base-devel clang gdb cmake make boost sdl3 sdl3_image sdl3_ttf fluidsynth pkgconf libxcb ncurses
				;;
			apt) # Debian Based =========================================================================================
				sudo apt update -y && sudo apt upgrade -y
				sudo apt install -y build-essential dkms linux-headers-$(uname -r) clang gdb make cmake libssl-dev libboost-all-dev libsdl3-dev libsdl3-image-dev \
									libsdl3-ttf-dev libxcb-randr0-dev libfluidsynth-dev pkg-config libxcb1-dev libncurses-dev
				;;
			dnf) # Fedora ===============================================================================================
				sudo dnf update -y
				sudo dnf install -y gcc gcc-c++ make clang gdb cmake clang-tools-extra boost boost-devel SDL3 SDL3_image SDL3_ttf SDL3-devel SDL3_image-devel \
									SDL3_ttf-devel fluidsynth-devel pkg-config libxcb-devel ncurses-devel
				;;
			*)
				echo "Unsupported distro. Supported distros are: Arch, EndeavourOS, Garuda, Manjaro, Ubuntu, Mint, Debian, Fedora."
				exit 1
				;;
		esac
		./setup-shader-tools.sh
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
