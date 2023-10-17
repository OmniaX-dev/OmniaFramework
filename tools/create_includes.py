# Note: This file is meant to be ran by CMake 
# 		and it should not be ran manually.
#
#		It uses hardcoded paths, relative to
#		the root of the project (the OmniaX directory)
#		therefore it is only meant to run directly 
#		from that directory and not form the
#		OmniaX/tools directory (in which it is located).

import os
import shutil
import os.path
import sys
from os import path

verbose = False

def run_fast_scandir(dir, ext):
    subfolders, files = [], []

    for f in os.scandir(dir):
        if f.is_dir():
            subfolders.append(f.path)
        if f.is_file():
            if os.path.splitext(f.name)[1].lower() in ext:
                files.append(f.path)


    for dir in list(subfolders):
        sf, f = run_fast_scandir(dir, ext)
        subfolders.extend(sf)
        files.extend(f)
    return subfolders, files

def install_includes(source_dir, include_dir, exception_dirs):
	_list = run_fast_scandir(source_dir, [".hpp", ".h", ".inl", ".cpp", ".c"])
	if path.exists(include_dir):
		if verbose:
			print("Removing old include directory...")
		shutil.rmtree(include_dir)
	os.mkdir(include_dir)

	for elem in _list[0]:
		elem = elem[len(source_dir) + 1:]
		elem = os.path.join(include_dir, elem)
		#elem = include_dir + "/" + elem
		os.mkdir(elem)
		if verbose:
			print("Creating Directory: " + elem)

	if verbose:
		print("\n")

	for elem in _list[1]:
		#new_elem = include_dir + "/" + elem[len(source_dir):]
		new_elem = os.path.join(include_dir, elem[len(source_dir) + 1:])
		found = True
		if elem.endswith(".c") or elem.endswith(".cpp"):
			found = False
			for exep in exception_dirs:
				if new_elem.startswith(exep):
					found = True
					break
		if found:
			shutil.copy(elem, new_elem)
			if verbose:
				print("Creating File: " + new_elem)


if __name__ == "__main__":
	if len(sys.argv) > 1 and sys.argv[1] == "--verbose":
		verbose = True
	#src_dir = "./src/omniax/"
	#inc_dir = "./Build/include"
	#excep_dirs = [inc_dir + "/vendor/GLAD", inc_dir + "/vendor/glm"]
	src_dir = os.path.join(".", "src")
	inc_dir = os.path.join(".", "Build", "include")
	excep_dirs = [os.path.join(inc_dir, "vendor", "GLAD"), os.path.join(inc_dir, "vendor", "glm"), os.path.join(inc_dir, "vendor", "clip"), os.path.join(inc_dir, "vendor", "MiniAudio")]
	install_includes(src_dir, inc_dir, excep_dirs)
    
