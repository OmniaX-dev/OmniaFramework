#!/usr/bin/env bash
# create_release.sh
#
# Assembles the release package in bin/OmniaFramework_<platform>/ from a finished build:
#   lib/        libostd, libogfx, libosnd (+ import libraries and runtime DLLs on Windows)
#   include/    every public header of ostd, ogfx and osnd, with their directory layout
#   licences/   third-party licences + OmniaFramework's own
#
# Usage: scripts/create_release.sh [--msys2-root DIR]
#   --msys2-root   Windows only: MSYS2's root (default: the running MSYS2, e.g. C:/msys64).
#                  The runtime DLLs bundled with the libraries are the ones from its ucrt64/bin.
# Normally run by ./build (release/debug, or -i); works from any directory.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$REPO_ROOT/bin"
MODULES="ostd ogfx osnd"

die() { printf '\033[1;31merror: %s\033[0m\n' "$*" >&2; exit 1; }

MSYS2_ROOT=""
while (($#)); do
	case "$1" in
		--msys2-root)
			[[ $# -ge 2 ]] || die "--msys2-root needs a path"
			MSYS2_ROOT="$2"
			shift 2
			;;
		*)
			die "unknown argument '$1'"
			;;
	esac
done

case "$(uname -s)" in
	Darwin)      PLATFORM="macos";   RELEASE_DIR="$BUILD_DIR/OmniaFramework_macos" ;;
	Linux*)      PLATFORM="linux";   RELEASE_DIR="$BUILD_DIR/OmniaFramework_linux64" ;;
	MINGW64_NT*) PLATFORM="windows"; RELEASE_DIR="$BUILD_DIR/OmniaFramework_w64" ;;
	*)           die "unsupported platform: $(uname -s)" ;;
esac

# Files that make up each library on this platform (relative to bin/).
library_files() { # library_files <module>
	case "$PLATFORM" in
		macos)   echo "lib$1.dylib" ;;
		linux)   echo "lib$1.so" ;;
		windows) echo "lib$1.dll lib$1.dll.a" ;;
	esac
}

# ===================================================== LIBRARIES =====================================================
missing=""
for module in $MODULES; do
	for file in $(library_files "$module"); do
		[[ -f "$BUILD_DIR/$file" ]] || missing="$missing $file"
	done
done
[[ -z "$missing" ]] || die "not built yet (bin/):$missing"

rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR/lib" "$RELEASE_DIR/include"
for module in $MODULES; do
	for file in $(library_files "$module"); do
		cp "$BUILD_DIR/$file" "$RELEASE_DIR/lib/"
	done
done

# Windows: bundle every DLL the libraries load from MSYS2's ucrt64 (compiler runtime, SDL3,
# FluidSynth and all of their own dependencies; ldd lists them recursively). System DLLs are
# left out, and so are our own libraries, which ldd may resolve to an older installed copy.
if [[ "$PLATFORM" == "windows" ]]; then
	[[ -n "$MSYS2_ROOT" ]] || MSYS2_ROOT="$(cygpath -m /)"
	ucrt_bin="$(cygpath -u "${MSYS2_ROOT%/}/ucrt64/bin" | tr '[:upper:]' '[:lower:]')"
	[[ -d "$ucrt_bin" ]] || die "$MSYS2_ROOT/ucrt64/bin doesn't exist (set MSYS2_ROOT)"
	own_dlls=""
	for module in $MODULES; do
		own_dlls="$own_dlls lib$module.dll"
	done

	dlls=""
	for module in $MODULES; do
		# ldd lines look like: "	SDL3.dll => /ucrt64/bin/SDL3.dll (0x7ff...)"
		dlls="$dlls $(ldd "$RELEASE_DIR/lib/lib$module.dll" | awk '$2 == "=>" { print $3 }')"
	done
	count=0
	for dll in $(printf '%s\n' $dlls | sort -u); do
		lower="$(printf '%s' "$dll" | tr '[:upper:]' '[:lower:]')"
		[[ "$lower" == "$ucrt_bin"/* ]] || continue
		name="$(basename "$lower")"
		[[ " $own_dlls " == *" $name "* ]] && continue
		cp "$dll" "$RELEASE_DIR/lib/"
		count=$((count + 1))
	done
	[[ $count -gt 0 ]] || die "ldd found no DLLs from $ucrt_bin; is MSYS2_ROOT right?"
	echo "Bundled $count runtime DLLs from $ucrt_bin"
fi
# ===================================================== LIBRARIES =====================================================





# ===================================================== HEADERS =====================================================
# Every header under src/<module>/, keeping the directory structure.
cd "$REPO_ROOT/src"
for module in $MODULES; do
	find "$module" -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.inl' \) | while IFS= read -r header; do
		mkdir -p "$RELEASE_DIR/include/$(dirname "$header")"
		cp "$header" "$RELEASE_DIR/include/$header"
	done
done
cd "$REPO_ROOT"
# ===================================================== HEADERS =====================================================





# ===================================================== LICENCES =====================================================
cp -r "$REPO_ROOT/licences" "$RELEASE_DIR/"
cp "$REPO_ROOT/LICENSE" "$RELEASE_DIR/licences/OmniaFramework-LICENCE.txt"
# ===================================================== LICENCES =====================================================

echo "Release package: ${RELEASE_DIR#"$REPO_ROOT"/}"
