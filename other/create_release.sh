#!/bin/bash

MSYS_ROOT=c:/msys64

set -e

RELEASE_DIR=

if [[ "$(uname)" == "Darwin" ]]; then
    RELEASE_DIR=../bin/OmniaFramework_macos
    mkdir -p "$RELEASE_DIR/lib"

    # macOS shared libraries (.dylib)
    cp ../bin/libostd.dylib "$RELEASE_DIR/lib"
    cp ../bin/libogfx.dylib "$RELEASE_DIR/lib"
elif [[ "$(uname -s)" == Linux* ]]; then
	RELEASE_DIR=../bin/OmniaFramework_linux64
	mkdir -p $RELEASE_DIR/lib
    cp ../bin/libostd.so $RELEASE_DIR/lib
    cp ../bin/libogfx.so $RELEASE_DIR/lib
elif [[ "$(uname -s)" == MINGW64_NT* ]]; then
	RELEASE_DIR=../bin/OmniaFramework_w64
	mkdir -p $RELEASE_DIR/lib
    cp ../bin/libostd.dll $RELEASE_DIR/lib
    cp ../bin/libostd.dll.a $RELEASE_DIR/lib
    cp ../bin/libogfx.dll $RELEASE_DIR/lib
    cp ../bin/libogfx.dll.a $RELEASE_DIR/lib

    cp $MSYS_ROOT/ucrt64/bin/libgcc_s_seh-1.dll $RELEASE_DIR/lib
    cp $MSYS_ROOT/ucrt64/bin/libstdc++-6.dll $RELEASE_DIR/lib
    cp $MSYS_ROOT/ucrt64/bin/libwinpthread-1.dll $RELEASE_DIR/lib
    cp $MSYS_ROOT/ucrt64/bin/SDL2.dll $RELEASE_DIR/lib
    cp $MSYS_ROOT/ucrt64/bin/SDL2_image.dll $RELEASE_DIR/lib
    cp $MSYS_ROOT/ucrt64/bin/SDL2_ttf.dll $RELEASE_DIR/lib
    cp $MSYS_ROOT/ucrt64/bin/libSDL2_gfx-1-0-0.dll $RELEASE_DIR/lib
fi

mkdir -p $RELEASE_DIR/include/ostd/vendor
mkdir -p $RELEASE_DIR/include/ostd/vendor/midifile
mkdir -p $RELEASE_DIR/include/ostd/data
mkdir -p $RELEASE_DIR/include/ostd/io
mkdir -p $RELEASE_DIR/include/ostd/math
mkdir -p $RELEASE_DIR/include/ostd/string
mkdir -p $RELEASE_DIR/include/ostd/utils
mkdir -p $RELEASE_DIR/include/ogfx/gui
mkdir -p $RELEASE_DIR/include/ogfx/render
mkdir -p $RELEASE_DIR/include/ogfx/resources
mkdir -p $RELEASE_DIR/include/ogfx/utils
find ../src/ostd  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ostd \;
find ../src/ostd/data  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ostd/data \;
find ../src/ostd/io  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ostd/io \;
find ../src/ostd/math  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ostd/math \;
find ../src/ostd/string  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ostd/string \;
find ../src/ostd/utils  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ostd/utils \;
find ../src/ostd/vendor  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ostd/vendor \;
find ../src/ostd/vendor/midifile  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ostd/vendor/midifile \;
find ../src/ogfx  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ogfx \;
find ../src/ogfx/gui  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ogfx/gui \;
find ../src/ogfx/render  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ogfx/render \;
find ../src/ogfx/resources  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ogfx/resources \;
find ../src/ogfx/utils  -maxdepth 1 -name "*.h*" -exec cp {} $RELEASE_DIR/include/ogfx/utils \;
cp -r ../src/ostd/vendor/nlohmann $RELEASE_DIR/include/ostd/vendor/
cp -r ../licences $RELEASE_DIR
cp ../LICENSE $RELEASE_DIR/licences/OmniaFramework-LICENCE.txt
