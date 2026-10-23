#!/usr/bin/env bash
# setup-shader-tools.sh
#
# Builds SDL_shadercross and fetches a pinned Microsoft DXC release into <repo>/tools/,
# so OmniaFramework's shaders can be regenerated. Not meant to be run by hand: ./build calls it.
#
# Everything lands in <repo>/tools/ (keep it in .gitignore):
#   tools/dxc/                  Microsoft DXC release (pinned tag + SHA-256)
#   tools/src/SDL_shadercross/  shadercross source + build dir
#   tools/shadercross/          shadercross install (bin/, lib/, include/)
#   tools/shader-tools.env      generated KEY=value file with paths, relative to the repo root
#
# Nothing is installed system-wide apart from the package-manager dependencies:
#   - no sudo for the build/install itself, no /etc edits, no PATH or env changes
#   - Linux: shadercross gets an RPATH to tools/shadercross/lib and tools/dxc/lib
#   - Windows: dxcompiler.dll/dxil.dll are copied next to shadercross.exe
#
# Platforms:
#   Linux        Arch-based (pacman). Other distros: install the deps listed below yourself.
#   MSYS2 UCRT64 must be run from the UCRT64 shell.
#   macOS        NOT supported. Microsoft ships no macOS DXC binaries, and shadercross needs
#                DXC to read HLSL at all. Regenerate shaders on Linux or Windows; normal builds
#                never need these tools because the compiled shaders are committed.
#
# Licensing: DXC's Microsoft components may not be redistributed standalone, which is why
# this script downloads them from Microsoft's GitHub releases instead of bundling them.

set -euo pipefail

# ---------------------------------------------------------------- config
SHADERCROSS_REPO="https://github.com/libsdl-org/SDL_shadercross.git"
SHADERCROSS_REF=""   # optional tag/commit to pin shadercross; empty = latest main

# Pinned DXC release. To upgrade: change the tag/files, run once with an empty hash,
# verify the printed hash, then paste it here. A different DXC version may produce
# different shader blobs, so upgrade deliberately.
DXC_TAG="v1.9.2609"
DXC_LINUX_FILE="linux_dxc_2026_09_28.x86_x64.tar.gz"
DXC_LINUX_SHA256="96faadc7f5c282d2ffda49804beb4c3ee38127bc252b723234e3c5cdf7aa39a1"
DXC_WINDOWS_FILE="dxc_2026_09_28.zip"   # check the exact asset name on the release page
DXC_WINDOWS_SHA256=""                   # empty: the script prints the hash and stops
# Optional PRIVATE mirror base URL (e.g. homelab behind auth; never public).
# The file name is appended. Downloads from it are checked against the same hash.
DXC_FALLBACK_BASE_URL=""

# ---------------------------------------------------------------- paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TOOLS_DIR="$REPO_ROOT/tools"
SRC_DIR="$TOOLS_DIR/src/SDL_shadercross"
BUILD_DIR="$SRC_DIR/build"
SC_PREFIX="$TOOLS_DIR/shadercross"
DXC_DIR="$TOOLS_DIR/dxc"
ENV_FILE="$TOOLS_DIR/shader-tools.env"

# ---------------------------------------------------------------- helpers
step() { printf '\033[1;36m==> %s\033[0m\n' "$*"; }
warn() { printf '\033[1;33mwarning: %s\033[0m\n' "$*" >&2; }
die()  { printf '\033[1;31merror: %s\033[0m\n' "$*" >&2; exit 1; }

TMP_DIR=""
cleanup() { [[ -n "$TMP_DIR" ]] && rm -rf "$TMP_DIR"; return 0; }
trap cleanup EXIT

# ---------------------------------------------------------------- platform
if [[ "$(uname)" == "Darwin" ]]; then
    die "macOS is not supported (no official DXC binaries for macOS). Regenerate shaders on Linux or Windows; see the header of this script."
elif [[ "$(uname -s)" == Linux* ]]; then
    PLATFORM="linux"
elif [[ "$(uname -s)" == MINGW64_NT* ]]; then
    [[ "${MSYSTEM:-}" == "UCRT64" ]] || die "run this from the MSYS2 UCRT64 shell (current MSYSTEM: ${MSYSTEM:-unset})"
    PLATFORM="windows"
else
    die "unsupported platform: $(uname -s)"
fi

# Native Windows CMake needs C:/... paths; elsewhere this is a no-op.
native_path() {
    if [[ "$PLATFORM" == "windows" ]]; then cygpath -m "$1"; else printf '%s' "$1"; fi
}

# ---------------------------------------------------------------- 1. dependencies
install_deps() {
    step "Installing dependencies"
    if [[ "$PLATFORM" == "linux" ]]; then
        if command -v pacman >/dev/null 2>&1; then
            sudo pacman -S --needed --noconfirm base-devel git cmake ninja curl sdl3 spirv-cross
        else
            warn "pacman not found; make sure these are installed: a C compiler, git, cmake, ninja, curl, SDL3 (dev), SPIRV-Cross (with spirv-cross-c-shared)"
        fi
    else
        pacman -S --needed --noconfirm git curl unzip \
            mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,sdl3,spirv-cross}
    fi
}

# ---------------------------------------------------------------- 2. pinned DXC
fetch_dxc() {
    local file hash
    if [[ "$PLATFORM" == "linux" ]]; then
        file="$DXC_LINUX_FILE";   hash="$DXC_LINUX_SHA256"
    else
        file="$DXC_WINDOWS_FILE"; hash="$DXC_WINDOWS_SHA256"
    fi

    local stamp="$DXC_TAG/$file"
    if [[ -f "$DXC_DIR/.dxc-version" && "$(cat "$DXC_DIR/.dxc-version")" == "$stamp" ]]; then
        step "DXC $DXC_TAG already in tools/dxc, skipping download"
        return
    fi

    step "Downloading DXC $DXC_TAG ($file)"
    TMP_DIR="$(mktemp -d)"
    local archive="$TMP_DIR/$file"
    local urls=("https://github.com/microsoft/DirectXShaderCompiler/releases/download/$DXC_TAG/$file")
    [[ -n "$DXC_FALLBACK_BASE_URL" ]] && urls+=("${DXC_FALLBACK_BASE_URL%/}/$file")

    local ok=0 url actual
    for url in "${urls[@]}"; do
        echo "    from $url"
        curl -fL --retry 3 -o "$archive" "$url" || { warn "download failed"; continue; }
        actual="$(sha256sum "$archive" | cut -d' ' -f1)"
        if [[ -z "$hash" ]]; then
            die "no SHA-256 pinned for $file. Downloaded file hashes to:
    $actual
Verify it (e.g. against a second download), then set it in this script."
        fi
        if [[ "$actual" == "$hash" ]]; then ok=1; break; fi
        warn "checksum mismatch from $url (got $actual)"
    done
    [[ $ok -eq 1 ]] || die "could not get a DXC archive matching the pinned SHA-256"

    step "Extracting DXC into tools/dxc"
    mkdir -p "$TMP_DIR/x"
    case "$file" in
        *.tar.gz) tar -xzf "$archive" -C "$TMP_DIR/x" ;;
        *.zip)    unzip -q "$archive" -d "$TMP_DIR/x" ;;
        *)        die "unknown archive type: $file" ;;
    esac

    # Find the archive's root regardless of whether it has a top-level folder.
    local marker root
    if [[ "$PLATFORM" == "linux" ]]; then
        marker="$(find "$TMP_DIR/x" -name libdxcompiler.so -print -quit)"
        [[ -n "$marker" ]] || die "libdxcompiler.so not found in the archive"
        root="$(dirname "$(dirname "$marker")")"                 # <root>/lib/libdxcompiler.so
    else
        marker="$(find "$TMP_DIR/x" -path '*/bin/x64/dxcompiler.dll' -print -quit)"
        [[ -n "$marker" ]] || die "bin/x64/dxcompiler.dll not found in the archive"
        root="$(dirname "$(dirname "$(dirname "$marker")")")"    # <root>/bin/x64/dxcompiler.dll
    fi

    rm -rf "$DXC_DIR"
    mkdir -p "$TOOLS_DIR"
    mv "$root" "$DXC_DIR"
    echo "$stamp" > "$DXC_DIR/.dxc-version"
    rm -rf "$TMP_DIR"; TMP_DIR=""
}

# Locates the DXC pieces shadercross's FindDirectXShaderCompiler.cmake looks for.
# We pass them as cache variables, so CMake uses our copy and never searches the system.
locate_dxc() {
    local hdr
    hdr="$(find "$DXC_DIR" -name dxcapi.h -print -quit)"
    [[ -n "$hdr" ]] || die "dxcapi.h not found in tools/dxc"
    DXC_INC="$(dirname "$hdr")"

    if [[ "$PLATFORM" == "linux" ]]; then
        DXC_LIB_DIR="$DXC_DIR/lib"
        for f in libdxcompiler.so libdxil.so; do
            [[ -f "$DXC_LIB_DIR/$f" ]] || die "tools/dxc/lib/$f is missing"
        done
    else
        DXC_LIB_DIR="$DXC_DIR/bin/x64"   # where the DLLs live
        for f in bin/x64/dxcompiler.dll bin/x64/dxil.dll lib/x64/dxcompiler.lib; do
            [[ -f "$DXC_DIR/$f" ]] || die "tools/dxc/$f is missing"
        done
    fi
}

# ---------------------------------------------------------------- 3. shadercross
fetch_shadercross() {
    if [[ -d "$SRC_DIR/.git" ]]; then
        step "Updating shadercross source"
        git -C "$SRC_DIR" fetch --tags origin
        if [[ -z "$SHADERCROSS_REF" ]]; then
            git -C "$SRC_DIR" checkout -q main
            git -C "$SRC_DIR" pull --ff-only
        fi
    else
        step "Cloning shadercross"
        mkdir -p "$(dirname "$SRC_DIR")"
        git clone "$SHADERCROSS_REPO" "$SRC_DIR"
    fi
    [[ -n "$SHADERCROSS_REF" ]] && git -C "$SRC_DIR" checkout -q "$SHADERCROSS_REF"
    # No submodules: they're only for SDLSHADERCROSS_VENDORED (which would build all of DXC).
    return 0
}

configure_shadercross() {
    local spirv_shared="$1"
    local args=(
        -S "$(native_path "$SRC_DIR")" -B "$(native_path "$BUILD_DIR")" -G Ninja
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX="$(native_path "$SC_PREFIX")"
        -DSDLSHADERCROSS_VENDORED=OFF
        -DSDLSHADERCROSS_DXC=ON
        -DSDLSHADERCROSS_CLI=ON
        -DSDLSHADERCROSS_INSTALL=ON
        -DSDLSHADERCROSS_TESTS=OFF
        -DSDLSHADERCROSS_SPIRVCROSS_SHARED="$spirv_shared"
        -DDirectXShaderCompiler_INCLUDE_PATH="$(native_path "$DXC_INC")"
    )
    if [[ "$PLATFORM" == "linux" ]]; then
        args+=(
            -DDirectXShaderCompiler_dxcompiler_LIBRARY="$DXC_LIB_DIR/libdxcompiler.so"
            -DDirectXShaderCompiler_dxil_LIBRARY="$DXC_LIB_DIR/libdxil.so"
            # Same depth for bin/ and lib/, so one RPATH works for the CLI and the library.
            '-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib;$ORIGIN/../../dxc/lib'
        )
    else
        args+=(
            -DDirectXShaderCompiler_dxcompiler_BINARY="$(native_path "$DXC_DIR/bin/x64/dxcompiler.dll")"
            -DDirectXShaderCompiler_dxcompiler_LIBRARY="$(native_path "$DXC_DIR/lib/x64/dxcompiler.lib")"
            -DDirectXShaderCompiler_dxil_BINARY="$(native_path "$DXC_DIR/bin/x64/dxil.dll")"
        )
    fi
    cmake "${args[@]}"
}

build_shadercross() {
    step "Configuring shadercross"
    rm -rf "$BUILD_DIR"   # always a clean configure: cached DXC paths must not go stale
    if ! configure_shadercross ON; then
        warn "configure with shared SPIRV-Cross failed, retrying with static SPIRV-Cross"
        rm -rf "$BUILD_DIR"
        configure_shadercross OFF || die "cmake configure failed"
    fi

    step "Building shadercross"
    cmake --build "$(native_path "$BUILD_DIR")" -j "$(nproc)"

    step "Installing shadercross into tools/shadercross"
    rm -rf "$SC_PREFIX"
    cmake --install "$(native_path "$BUILD_DIR")"

    if [[ "$PLATFORM" == "windows" ]]; then
        # Windows has no RPATH: DLLs are found next to the .exe. SDL3 and SPIRV-Cross DLLs
        # come from /ucrt64/bin, which is on PATH inside the UCRT64 shell.
        cp "$DXC_DIR/bin/x64/dxcompiler.dll" "$DXC_DIR/bin/x64/dxil.dll" "$SC_PREFIX/bin/"
    fi
}

verify_shadercross() {
    step "Verifying shadercross"
    local bin="$SC_PREFIX/bin/shadercross"
    [[ "$PLATFORM" == "windows" ]] && bin="$bin.exe"
    [[ -x "$bin" ]] || die "$bin was not installed"

    if [[ "$PLATFORM" == "linux" ]] && ldd "$bin" | grep -q 'not found'; then
        ldd "$bin" | grep 'not found' >&2
        die "shadercross has unresolved libraries (see above)"
    fi
    # 127 = the loader couldn't start it (missing .so/.dll). Any other code means it ran.
    local rc=0
    "$bin" --help >/dev/null 2>&1 || rc=$?
    [[ $rc -ne 127 ]] || die "shadercross fails to start (missing shared libraries?)"
    SHADERCROSS_BIN="$bin"
}

# ---------------------------------------------------------------- 4. env file for ./build
write_env() {
    local rel_sc="${SHADERCROSS_BIN#"$REPO_ROOT"/}"
    local rel_dxc="${DXC_LIB_DIR#"$REPO_ROOT"/}"
    cat > "$ENV_FILE" <<EOF
# Generated by scripts/setup-shader-tools.sh - do not edit. Paths are relative to the repo root.
PLATFORM=$PLATFORM
SHADERCROSS=$rel_sc
DXC_LIB_DIR=$rel_dxc
DXC_TAG=$DXC_TAG
EOF
}

# ---------------------------------------------------------------- main
install_deps
fetch_dxc
locate_dxc
fetch_shadercross
build_shadercross
verify_shadercross
write_env

printf '\033[1;32m%s\033[0m\n' "Shader tools ready ($PLATFORM)"
echo "  shadercross: ${SHADERCROSS_BIN#"$REPO_ROOT"/}"
echo "  DXC:         ${DXC_LIB_DIR#"$REPO_ROOT"/} ($DXC_TAG)"
echo "  env file:    ${ENV_FILE#"$REPO_ROOT"/}"
