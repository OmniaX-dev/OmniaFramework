#!/bin/bash

MSYS_ROOT=c:/msys64

set -e

RELEASE_DIR=

if [[ "$(uname)" == "Darwin" ]]; then
    RELEASE_DIR=../bin/OmniaFramework_macos

    # Remove old versions
    sudo rm -rf /usr/local/include/ostd 2> /dev/null
    sudo rm -rf /usr/local/include/ogfx 2> /dev/null
    sudo rm -f  /usr/local/lib/libostd.dylib 2> /dev/null
    sudo rm -f  /usr/local/lib/libogfx.dylib 2> /dev/null

    # Install headers
    sudo cp -r "$RELEASE_DIR/include/ostd" /usr/local/include/
    sudo cp -r "$RELEASE_DIR/include/ogfx" /usr/local/include/

    # Install libraries
    sudo cp "$RELEASE_DIR/lib/libostd.dylib" /usr/local/lib/
    sudo cp "$RELEASE_DIR/lib/libogfx.dylib" /usr/local/lib/
elif [[ "$(uname -s)" == Linux* ]]; then
	RELEASE_DIR=../bin/OmniaFramework_linux64
	sudo rm -rf /usr/include/ostd 2> /dev/null
	sudo rm -rf /usr/include/ogfx 2> /dev/null
	sudo rm -rf /usr/lib/libostd.so 2> /dev/null
	sudo rm -rf /usr/lib/libogfx.so 2> /dev/null
	sudo cp -r $RELEASE_DIR/include/ostd /usr/include/
	sudo cp -r $RELEASE_DIR/include/ogfx /usr/include/
	sudo cp -r $RELEASE_DIR/lib/libostd.so /usr/lib/
	sudo cp -r $RELEASE_DIR/lib/libogfx.so /usr/lib/
elif [[ "$(uname -s)" == MINGW64_NT* ]]; then
	RELEASE_DIR=../bin/OmniaFramework_w64
	rm -rf $MSYS_ROOT/ucrt64/include/ostd 2> /dev/null
	rm -rf $MSYS_ROOT/ucrt64/include/ogfx 2> /dev/null
	rm -rf $MSYS_ROOT/ucrt64/bin/libostd.dll 2> /dev/null
	rm -rf $MSYS_ROOT/ucrt64/lib/libostd.dll.a 2> /dev/null
	rm -rf $MSYS_ROOT/ucrt64/bin/libogfx.dll 2> /dev/null
	rm -rf $MSYS_ROOT/ucrt64/lib/libogfx.dll.a 2> /dev/null
	cp -r $RELEASE_DIR/include/ostd $MSYS_ROOT/ucrt64/include/
	cp -r $RELEASE_DIR/include/ogfx $MSYS_ROOT/ucrt64/include/
	cp -r $RELEASE_DIR/lib/libostd.dll $MSYS_ROOT/ucrt64/bin/
	cp -r $RELEASE_DIR/lib/libostd.dll.a $MSYS_ROOT/ucrt64/lib/
	cp -r $RELEASE_DIR/lib/libogfx.dll $MSYS_ROOT/ucrt64/bin/
	cp -r $RELEASE_DIR/lib/libogfx.dll.a $MSYS_ROOT/ucrt64/lib/
fi
