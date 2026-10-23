# Fish completions for OmniaFramework's ./build script (see ./build --help).
#
# Loaded from .fish-complete/ through fish_complete_path. The completions are tied to this
# checkout's build script by its absolute path, so other projects' "build" scripts are left
# alone. A wildcard pattern like `complete -p '*/OmniaFramework/build'` would be the obvious
# way, but fish 4.9 doesn't match wildcards in -p patterns, only exact paths.

set -l build_script (path resolve (status dirname)/../build)

# ===================================================== HELPERS =====================================================
# Repo root of the build script on the command line being completed.
function __omnia_build_root
    path resolve (path dirname -- (commandline -xpc)[1])
end

function __omnia_build_targets
    printf '%s\n' \
        release\t'Clean Release build + package' \
        debug\t'Clean Debug build + package' \
        run\t'Incremental build, then run' \
        dbg\t'Incremental build, then debug' \
        dependencies\t'Install the build dependencies' \
        shaders\t'Regenerate the embedded shaders'
end

# CMake targets from the configured Makefile, or the known ones before the first configure.
function __omnia_build_cmake_targets
    set -l root (__omnia_build_root)
    if test -f $root/bin/Makefile
        make -C $root/bin help 2>/dev/null \
            | string replace -rf '^\.\.\. (\S+).*' '$1' \
            | string match -rv '\.(o|i|s)$|^(all|depend|edit_cache|rebuild_cache)$'
    else
        printf '%s\n' ostd osnd ogfx ostd_test blur_example ogfx_shaders
    end
end

# Words after -v/--variables are VAR=VALUE overrides, up to the next word that isn't one.
function __omnia_build_in_variables
    set -l in_variables 0
    for token in (commandline -xpc)[2..]
        switch $token
            case -v --variables
                set in_variables 1
            case '-*'
                set in_variables 0
            case '*=*'
            case '*'
                set in_variables 0
        end
    end
    test $in_variables = 1
end

# True until a build target was given, with -t or as a plain word.
function __omnia_build_needs_target
    set -l skip_next 0
    for token in (commandline -xpc)[2..]
        if test $skip_next = 1
            set skip_next 0
            continue
        end
        switch $token
            case -c --cmake-target
                set skip_next 1
            case -t --build-target '--build-target=*'
                return 1
            case '-*' '*=*'
            case '*'
                return 1
        end
    end
    return 0
end

function __omnia_build_variables
    set -l token (commandline -ct)
    if string match -qr '^(?<name>[A-Z0-9_]+)=(?<value>.*)$' -- $token
        switch $name
            case INSTALL_DIR MSYS2_ROOT DXC_LIB_DIR
                __fish_complete_directories $value | string replace -r '^' "$name="
            case SHADERCROSS
                __fish_complete_path $value | string replace -r '^' "$name="
            case NO_DXIL SKIP_TABS_FIX SKIP_SHADER_TOOLS
                printf '%s\n' $name=true $name=false
        end
        return
    end

    # Variables not given yet.
    set -l given (commandline -xpc | string match -r '^[A-Z0-9_]+(?==)')
    for entry in \
            INSTALL_DIR=\t'Install prefix (-i)' \
            MSYS2_ROOT=\t'Windows: MSYS2 root' \
            SHADERCROSS=\t'shadercross executable (shaders)' \
            DXC_LIB_DIR=\t'DXC lib directory (shaders)' \
            NO_DXIL=\t'Skip DXIL (shaders)' \
            SKIP_TABS_FIX=\t'Skip fix_tabs.py' \
            SKIP_SHADER_TOOLS=\t'Skip shader tools (dependencies)'
        contains -- (string split -f1 = -- $entry) $given; or echo $entry
    end
end
# ===================================================== HELPERS =====================================================





# ===================================================== COMPLETIONS =====================================================
complete -p $build_script -f
complete -p $build_script -s h -l help -d 'Show usage'
complete -p $build_script -s i -l install -d 'Also install (no target, release or debug)'
complete -p $build_script -s c -l cmake-target -x -a '(__omnia_build_cmake_targets)' -d 'Build only this CMake target'
complete -p $build_script -s t -l build-target -x -a '(__omnia_build_targets)' -d 'What to build/do'
complete -p $build_script -s v -l variables -d 'VAR=VALUE overrides follow'
complete -p $build_script -n __omnia_build_in_variables -a '(__omnia_build_variables)'
complete -p $build_script -n __omnia_build_needs_target -a '(__omnia_build_targets)'
# ===================================================== COMPLETIONS =====================================================
