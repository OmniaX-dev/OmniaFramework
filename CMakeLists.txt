#Variables
#-----------------------------------------------------------------------------------------
set(PROJ_NAME omnia-framework)
set(MAJOR_VER 0)
set(MINOR_VER 3)
#-----------------------------------------------------------------------------------------

#Setup
#-----------------------------------------------------------------------------------------
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.18)
project(${PROJ_NAME} LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
file(STRINGS "./other/build.nr" BUILD_NUMBER)
#-----------------------------------------------------------------------------------------

message(STATUS "Building ${PROJ_NAME} ${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message(STATUS "Debug Configuration")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "Release Configuration")
endif()

#Sources
#-----------------------------------------------------------------------------------------
list(APPEND INCLUDE_DIRS
	${CMAKE_CURRENT_LIST_DIR}/src
	${CMAKE_CURRENT_LIST_DIR}/src/ostd
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor/midifile
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor/nlohmann
)
list(APPEND OSTD_SOURCE_FILES
	# MidiFile
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor/midifile/Binasc.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor/midifile/MidiEvent.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor/midifile/MidiEventList.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor/midifile/MidiFile.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor/midifile/MidiMessage.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor/midifile/Options.cpp

	# data
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/data/BaseObject.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/data/Color.cpp

	# io
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Console.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Errors.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/File.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/FileSystem.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Json.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Keyboard.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Logger.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Midi.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/OutputHandlers.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Serial.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Memory.cpp

	# math
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/Geometry.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/ShuntingYard.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/Random.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/SineWave.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/Spline.cpp

	# string
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/string/String.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/string/TextStyleParser.cpp

	# utils
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Logic.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Hash.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/PathFinder.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/QuadTree.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Signals.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Time.cpp
)
list(APPEND OGFX_SOURCE_FILES
	# sdl3_gfx
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/vendor/sdl3_gfx/SDL3_framerate.c
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/vendor/sdl3_gfx/SDL3_gfxPrimitives.c
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/vendor/sdl3_gfx/SDL3_imageFilter.c
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/vendor/sdl3_gfx/SDL3_rotozoom.c

	# gui
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/RawTextInput.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/Window.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/WindowOutputHandler.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/Widgets.cpp

	# render
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/render/BasicRenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/render/FontUtils.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/render/PixelRenderer.cpp

	# resources
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/resources/Image.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/resources/UbuntuMonoRegularTTF.cpp

	# utils
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/utils/Animation.cpp
)
list(APPEND TEST_SOURCE_FILES
	# ${CMAKE_CURRENT_LIST_DIR}/src/test/GraphicsWindowTest.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/test/GuiTest.cpp
)
#-----------------------------------------------------------------------------------------

#Targets
#-----------------------------------------------------------------------------------------
set(OMNIA_STD_LIB ostd)
set(OMNIA_GFX_LIB ogfx)
set(TEST_TARGET ostd_test)
add_library(${OMNIA_STD_LIB} SHARED ${OSTD_SOURCE_FILES})
add_library(${OMNIA_GFX_LIB} SHARED ${OGFX_SOURCE_FILES})
add_executable(${TEST_TARGET} ${TEST_SOURCE_FILES})

target_compile_definitions(${OMNIA_STD_LIB} PUBLIC BUILD_CONFIG="${CMAKE_BUILD_TYPE}" BUILD_NR=${BUILD_NUMBER} MAJ_V=${MAJOR_VER} MIN_V=${MINOR_VER} VERSION_STR="${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
target_compile_definitions(${OMNIA_GFX_LIB} PUBLIC BUILD_CONFIG="${CMAKE_BUILD_TYPE}" BUILD_NR=${BUILD_NUMBER} MAJ_V=${MAJOR_VER} MIN_V=${MINOR_VER} VERSION_STR="${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
target_include_directories(${OMNIA_STD_LIB} PUBLIC ${INCLUDE_DIRS})
target_include_directories(${OMNIA_GFX_LIB} PUBLIC ${INCLUDE_DIRS})
target_include_directories(${TEST_TARGET} PUBLIC ${INCLUDE_DIRS})

if (WIN32)
	set(MSYS2_UCRT64 "C:/msys64/ucrt64")
	target_include_directories(${OMNIA_STD_LIB} PRIVATE
								"${MSYS2_UCRT64}/include"
								"${MSYS2_UCRT64}/include/c++/13.2.0"
								"${MSYS2_UCRT64}/include/c++/13.2.0/x86_64-w64-mingw32"
				)
	set(MSYS2_UCRT64 "C:/msys64/ucrt64")
	target_include_directories(${OMNIA_GFX_LIB} PRIVATE
								"${MSYS2_UCRT64}/include"
								"${MSYS2_UCRT64}/include/c++/13.2.0"
								"${MSYS2_UCRT64}/include/c++/13.2.0/x86_64-w64-mingw32"
				)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-O3 -MMD -MP -Wall -ggdb -fsanitize=address)
	target_compile_definitions(${OMNIA_STD_LIB} PUBLIC OX_DEBUG_BUILD)
	target_compile_definitions(${OMNIA_GFX_LIB} PUBLIC OX_DEBUG_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_options(-MMD -MP -Wall)
	target_compile_definitions(${OMNIA_STD_LIB} PUBLIC OX_RELEASE_BUILD)
	target_compile_definitions(${OMNIA_GFX_LIB} PUBLIC OX_RELEASE_BUILD)
endif()
#-----------------------------------------------------------------------------------------

#Linking
#-----------------------------------------------------------------------------------------
if (WIN32)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC mingw32)
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC mingw32)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC SDL3 SDL3_ttf SDL3_image)
endif (WIN32)

target_link_libraries(${OMNIA_GFX_LIB} PUBLIC ${OMNIA_STD_LIB})
target_link_libraries(${TEST_TARGET} PUBLIC ${OMNIA_STD_LIB})
target_link_libraries(${TEST_TARGET} PUBLIC ${OMNIA_GFX_LIB})

if (APPLE)
	cmake_policy(SET CMP0167 OLD)
	find_package(Boost REQUIRED COMPONENTS regex)
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC Boost::regex)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDL3 REQUIRED sdl3)
	pkg_check_modules(SDL3_IMAGE REQUIRED SDL3_image)
	pkg_check_modules(SDL3_TTF REQUIRED SDL3_ttf)
	target_include_directories(${OMNIA_GFX_LIB} PUBLIC
					${SDL3_INCLUDE_DIRS}
					${SDL3_IMAGE_INCLUDE_DIRS}
					${SDL3_TTF_INCLUDE_DIRS}
					${SDL3_GFX_INCLUDE_DIRS}
	)
	target_link_directories(${OMNIA_GFX_LIB} PUBLIC
					${SDL3_LIBRARY_DIRS}
					${SDL3_IMAGE_LIBRARY_DIRS}
					${SDL3_TTF_LIBRARY_DIRS}
					${SDL3_GFX_LIBRARY_DIRS}
	)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC
					${SDL3_LIBRARIES}
					${SDL3_IMAGE_LIBRARIES}
					${SDL3_TTF_LIBRARIES}
					${SDL3_GFX_LIBRARIES}
	)
elseif (UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN',-rpath,/usr/lib,-rpath,/usr/local/lib")
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC tinfo boost_regex)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC xcb xcb-randr)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC SDL3 SDL3_ttf SDL3_image)
endif ()
#-----------------------------------------------------------------------------------------
