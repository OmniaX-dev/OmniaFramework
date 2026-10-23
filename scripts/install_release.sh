#!/usr/bin/env bash
# install_release.sh
#
# Installs the release package made by create_release.sh (bin/OmniaFramework_<platform>/):
# removes the previously installed headers and libraries, then copies the new ones.
#
# Usage: scripts/install_release.sh [--prefix DIR] [--msys2-root DIR]
#   --prefix       install into DIR/include and DIR/lib (Windows: DLLs into DIR/bin)
#   --msys2-root   Windows only: MSYS2's root (default: the running MSYS2, e.g. C:/msys64)
#
# Default locations without --prefix:
#   Linux     /usr/include, /usr/lib
#   macOS     /usr/local/include, <Homebrew prefix>/lib (/opt/homebrew or /usr/local)
#   Windows   <MSYS2 root>/ucrt64/include, ucrt64/bin (DLLs), ucrt64/lib (import libraries)
# sudo is only used when a destination isn't writable by the current user.
# Normally run by ./build -i; works from any directory.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
MODULES="ostd ogfx osnd"

die() { printf '\033[1;31merror: %s\033[0m\n' "$*" >&2; exit 1; }

PREFIX=""
MSYS2_ROOT=""
while (($#)); do
	case "$1" in
		--prefix|--msys2-root)
			[[ $# -ge 2 && -n "$2" ]] || die "$1 needs a path"
			if [[ "$1" == "--prefix" ]]; then PREFIX="${2%/}"; else MSYS2_ROOT="${2%/}"; fi
			shift 2
			;;
		*)
			die "unknown argument '$1'"
			;;
	esac
done

# INCLUDE_DIR gets the headers, LIB_DIR the libraries; on Windows the DLLs go to BIN_DIR
# and the import libraries (.dll.a) to LIB_DIR.
case "$(uname -s)" in
	Darwin)
		PLATFORM="macos"
		RELEASE_DIR="$REPO_ROOT/bin/OmniaFramework_macos"
		if [[ -n "$PREFIX" ]]; then
			INCLUDE_DIR="$PREFIX/include"; LIB_DIR="$PREFIX/lib"
		else
			if [[ "$(uname -m)" == "arm64" ]]; then brew_prefix="/opt/homebrew"; else brew_prefix="/usr/local"; fi
			INCLUDE_DIR="/usr/local/include"; LIB_DIR="$brew_prefix/lib"
		fi
		;;
	Linux*)
		PLATFORM="linux"
		RELEASE_DIR="$REPO_ROOT/bin/OmniaFramework_linux64"
		if [[ -n "$PREFIX" ]]; then
			INCLUDE_DIR="$PREFIX/include"; LIB_DIR="$PREFIX/lib"
		else
			INCLUDE_DIR="/usr/include"; LIB_DIR="/usr/lib"
		fi
		;;
	MINGW64_NT*)
		PLATFORM="windows"
		RELEASE_DIR="$REPO_ROOT/bin/OmniaFramework_w64"
		if [[ -z "$PREFIX" ]]; then
			[[ -n "$MSYS2_ROOT" ]] || MSYS2_ROOT="$(cygpath -m /)"
			PREFIX="${MSYS2_ROOT%/}/ucrt64"
			[[ -d "$PREFIX" ]] || die "$PREFIX doesn't exist (set MSYS2_ROOT)"
		fi
		INCLUDE_DIR="$PREFIX/include"; LIB_DIR="$PREFIX/lib"; BIN_DIR="$PREFIX/bin"
		;;
	*)
		die "unsupported platform: $(uname -s)"
		;;
esac

[[ -d "$RELEASE_DIR/lib" && -d "$RELEASE_DIR/include" ]] \
	|| die "no release package in ${RELEASE_DIR#"$REPO_ROOT"/} (run ./build release first)"

# Use sudo only if some destination (or its nearest existing parent) isn't writable.
SUDO=""
if [[ "$PLATFORM" != "windows" ]]; then
	for dir in "$INCLUDE_DIR" "$LIB_DIR"; do
		while [[ ! -e "$dir" ]]; do dir="$(dirname "$dir")"; done
		[[ -w "$dir" ]] || SUDO="sudo"
	done
fi
as_needed() { ${SUDO:+$SUDO} "$@"; }

# ===================================================== INSTALL =====================================================
as_needed mkdir -p "$INCLUDE_DIR" "$LIB_DIR"
[[ "$PLATFORM" != "windows" ]] || mkdir -p "$BIN_DIR"

for module in $MODULES; do
	[[ -d "$RELEASE_DIR/include/$module" ]] || die "the release package has no include/$module"
	as_needed rm -rf "$INCLUDE_DIR/$module"
	as_needed cp -r "$RELEASE_DIR/include/$module" "$INCLUDE_DIR/"

	case "$PLATFORM" in
		macos)
			as_needed rm -f "$LIB_DIR/lib$module.dylib"
			as_needed cp "$RELEASE_DIR/lib/lib$module.dylib" "$LIB_DIR/"
			;;
		linux)
			as_needed rm -f "$LIB_DIR/lib$module.so"
			as_needed cp "$RELEASE_DIR/lib/lib$module.so" "$LIB_DIR/"
			;;
		windows)
			rm -f "$BIN_DIR/lib$module.dll" "$LIB_DIR/lib$module.dll.a"
			cp "$RELEASE_DIR/lib/lib$module.dll" "$BIN_DIR/"
			cp "$RELEASE_DIR/lib/lib$module.dll.a" "$LIB_DIR/"
			;;
	esac
done
# ===================================================== INSTALL =====================================================

echo "Installed OmniaFramework:"
echo "  headers:   $INCLUDE_DIR/{$(echo $MODULES | tr ' ' ',')}"
if [[ "$PLATFORM" == "windows" ]]; then
	echo "  DLLs:      $BIN_DIR"
	echo "  libraries: $LIB_DIR"
else
	echo "  libraries: $LIB_DIR"
fi
