#!/bin/bash

if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	sudo rm -r /usr/include/ostd
	sudo rm -r /usr/lib/libostd.so
	sudo cp -r ./Release/include/ostd /usr/include/
	sudo cp -r ./Release/lib/libostd.so /usr/lib/
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
	rm -r C:/msys64/ucrt64/include/ostd
	rm -r C:/msys64/ucrt64/bin/libostd.dll
	rm -r C:/msys64/ucrt64/lib/libostd.dll.a
	cp -r ./Release/include/ostd C:/msys64/ucrt64/include/
	cp -r ./Release/lib/libostd.dll C:/msys64/ucrt64/bin/
	cp -r ./Release/lib/libostd.dll.a C:/msys64/ucrt64/lib/
fi
