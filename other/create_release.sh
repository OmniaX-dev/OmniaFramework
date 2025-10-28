#!/bin/bash

MSYS_ROOT=c:/msys64

set -e

RELEASE_DIR=

if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	RELEASE_DIR=../bin/OmniaFramework_linux64
	mkdir -p $RELEASE_DIR/lib
    cp ../bin/libostd.so $RELEASE_DIR/lib
    cp ../bin/libogfx.so $RELEASE_DIR/lib
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
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
mkdir -p $RELEASE_DIR/include/ogfx/
find ../src/ostd  -maxdepth 1 -name "*.hpp" -exec cp {} $RELEASE_DIR/include/ostd \;
find ../src/ogfx  -maxdepth 1 -name "*.hpp" -exec cp {} $RELEASE_DIR/include/ogfx \;
find ../src/ostd/vendor  -maxdepth 1 -name "*.hpp" -exec cp {} $RELEASE_DIR/include/ostd/vendor \;
cp -r ../licences $RELEASE_DIR
cp ../LICENSE $RELEASE_DIR/licences/OmniaFramework-LICENCE.txt
