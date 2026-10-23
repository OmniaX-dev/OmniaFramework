#!/usr/bin/env fish
# install-shadercross.fish
# 1. Builds and installs SDL_shadercross (CLI + library) into /usr/local on an Arch-based system,
#    and makes sure the dynamic linker can find /usr/local/lib.
# 2. Downloads a pinned, checksum-verified Microsoft DXC release into ~/tools/dxc (never system-wide)
#    and sets the universal fish variable DXC_LIB_DIR for OmniaFramework's build_shaders.py.
#
# Usage: ./install-shadercross.fish [source-dir]
#   source-dir defaults to ~/src/SDL_shadercross. Re-running it updates and rebuilds.

# ---------------------------------------------------------------- config
set -l repo_url https://github.com/libsdl-org/SDL_shadercross.git
set -l src_dir ~/src/SDL_shadercross
if set -q argv[1]
    set src_dir $argv[1]
end
set -l build_dir $src_dir/build

# Pinned DXC release. To upgrade: change tag + file, download once, put the new hash here.
# Note: a different DXC version may produce different shader blobs (git diffs on regenerate).
set -l dxc_tag    v1.9.2609
set -l dxc_file   linux_dxc_2026_09_28.x86_x64.tar.gz
set -l dxc_sha256 96faadc7f5c282d2ffda49804beb4c3ee38127bc252b723234e3c5cdf7aa39a1
set -l dxc_url    https://github.com/microsoft/DirectXShaderCompiler/releases/download/$dxc_tag/$dxc_file
# Optional private fallback (e.g. homelab, NOT public - Microsoft's license forbids standalone
# redistribution). Leave empty to disable. Checked against the same hash.
set -l dxc_fallback_url ""
set -l dxc_dir ~/tools/dxc

# ---------------------------------------------------------------- helpers
function step
    set_color --bold cyan
    echo "==> $argv"
    set_color normal
end

function fail
    set_color --bold red
    echo "error: $argv" >&2
    set_color normal
    exit 1
end

# ---------------------------------------------------------------- 1. dependencies
#    spirv-cross              -> libspirv-cross-c-shared, needed for SPIR-V -> MSL/HLSL
#    directx-shader-compiler  -> Arch's DXC build, used by shadercross for HLSL input
step "Installing build dependencies"
sudo pacman -S --needed --noconfirm base-devel git cmake curl sdl3 spirv-cross directx-shader-compiler
or fail "pacman failed"

# ---------------------------------------------------------------- 2. clone or update
if test -d $src_dir/.git
    step "Updating existing clone in $src_dir"
    git -C $src_dir pull --ff-only; or fail "git pull failed"
else
    step "Cloning into $src_dir"
    mkdir -p (path dirname $src_dir)
    git clone $repo_url $src_dir; or fail "git clone failed"
end
git -C $src_dir submodule update --init --recursive; or fail "submodule update failed"

# ---------------------------------------------------------------- 3. build + install
step "Configuring"
cmake -S $src_dir -B $build_dir -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
or fail "cmake configure failed"

step "Building"
cmake --build $build_dir -j (nproc); or fail "build failed"

step "Installing to /usr/local (needs sudo)"
sudo cmake --install $build_dir; or fail "install failed"

# ---------------------------------------------------------------- 4. linker path
# Arch doesn't search /usr/local/lib by default, and the installed binary has no RPATH.
if not grep -rqsx '/usr/local/lib' /etc/ld.so.conf /etc/ld.so.conf.d/
    step "Adding /usr/local/lib to the linker search path"
    echo /usr/local/lib | sudo tee /etc/ld.so.conf.d/usr-local.conf >/dev/null
    or fail "could not write /etc/ld.so.conf.d/usr-local.conf"
end
step "Refreshing linker cache"
sudo ldconfig; or fail "ldconfig failed"

step "Verifying shadercross"
if not command -q shadercross
    fail "shadercross is not on PATH (is /usr/local/bin in \$PATH?)"
end
if not shadercross --help >/dev/null 2>&1
    fail "shadercross is installed but fails to run, try: ldd (command -s shadercross)"
end

# ---------------------------------------------------------------- 5. pinned Microsoft DXC
if test -f $dxc_dir/.dxc-tag; and test (cat $dxc_dir/.dxc-tag) = $dxc_tag
    step "DXC $dxc_tag already installed in $dxc_dir, skipping download"
else
    step "Downloading DXC $dxc_tag"
    set -l tmp (mktemp -d); or fail "mktemp failed"
    set -l archive $tmp/$dxc_file

    set -l ok 0
    for url in $dxc_url $dxc_fallback_url
        test -z "$url"; and continue
        echo "    from $url"
        if curl -fL --retry 3 -o $archive $url
            if echo "$dxc_sha256  $archive" | sha256sum -c --quiet -
                set ok 1
                break
            end
            echo "    checksum mismatch, trying next source" >&2
        end
    end
    if test $ok -ne 1
        rm -rf $tmp
        fail "could not download a DXC archive matching the pinned SHA-256"
    end

    step "Extracting DXC to $dxc_dir"
    mkdir -p $tmp/x
    tar -xzf $archive -C $tmp/x; or begin; rm -rf $tmp; fail "tar failed"; end

    # Locate the archive's root regardless of whether it has a top-level folder.
    set -l lib (find $tmp/x -name libdxcompiler.so -print -quit)
    if test -z "$lib"
        rm -rf $tmp
        fail "libdxcompiler.so not found in the archive"
    end
    set -l root (path dirname (path dirname $lib))

    rm -rf $dxc_dir
    mkdir -p (path dirname $dxc_dir)
    mv $root $dxc_dir; or begin; rm -rf $tmp; fail "could not move DXC into place"; end
    echo $dxc_tag > $dxc_dir/.dxc-tag
    rm -rf $tmp
end

for f in libdxcompiler.so libdxil.so
    test -f $dxc_dir/lib/$f; or fail "$dxc_dir/lib/$f is missing"
end

# Universal variable: persists across all fish sessions, read by build_shaders.py.
set -Ux DXC_LIB_DIR $dxc_dir/lib

step "Verifying DXC"
if test -x $dxc_dir/bin/dxc
    env LD_LIBRARY_PATH=$dxc_dir/lib $dxc_dir/bin/dxc --version 2>/dev/null | head -n 1
    or fail "DXC binary fails to run, try: ldd $dxc_dir/lib/libdxcompiler.so"
end

# ---------------------------------------------------------------- done
set_color --bold green
echo "shadercross: "(command -s shadercross)
echo "DXC:         $dxc_dir ($dxc_tag)"
echo "DXC_LIB_DIR: $DXC_LIB_DIR"
set_color normal
echo "Open a new shell (or this one already has it) and regenerate shaders with:"
echo "    python3 shaders/build_shaders.py --shadercross shadercross"
echo "(check build_shaders.py --help for the exact flags)"
