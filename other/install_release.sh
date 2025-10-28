#!/bin/bash

MSYS_ROOT=c:/msys64

set -e

RELEASE_DIR=

if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	RELEASE_DIR=../bin/OmniaFramework_linux64
	sudo rm -rf /usr/include/ostd 2> /dev/null
	sudo rm -rf /usr/include/ogfx 2> /dev/null
	sudo rm -rf /usr/lib/libostd.so 2> /dev/null
	sudo rm -rf /usr/lib/libogfx.so 2> /dev/null
	sudo cp -r $RELEASE_DIR/include/ostd /usr/include/
	sudo cp -r $RELEASE_DIR/include/ogfx /usr/include/
	sudo cp -r $RELEASE_DIR/lib/libostd.so /usr/lib/
	sudo cp -r $RELEASE_DIR/lib/libogfx.so /usr/lib/
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
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
