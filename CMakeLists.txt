#Variables
#-----------------------------------------------------------------------------------------
set(PROJ_NAME omnia-framework)
set(MAJOR_VER 0)
set(MINOR_VER 2)
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

	${CMAKE_CURRENT_LIST_DIR}/src/ostd/BaseObject.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Color.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Console.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Errors.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/File.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/FileSystem.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Geometry.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Json.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Keyboard.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Logger.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Logic.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/md5.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Midi.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/OutputHandlers.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/PathFinder.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/QuadTree.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/ShuntingYard.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Signals.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Random.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Serial.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/SineWave.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Spline.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/StringEditor.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/String.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/TextStyleParser.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Time.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Utils.cpp
)
list(APPEND OGFX_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/FontUtils.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/WindowBase.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/BasicRenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/RawTextInput.cpp
				${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Image.cpp
)
list(APPEND TEST_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/test.cpp
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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-O3 -MMD -MP -Wall -ggdb -fsanitize=address)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_options(-MMD -MP -Wall)
endif()
#-----------------------------------------------------------------------------------------

#Linking
#-----------------------------------------------------------------------------------------
if (WIN32)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC mingw32)
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC mingw32)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC SDL2main SDL2 SDL2_gfx SDL2_ttf SDL2_image)
endif (WIN32)

target_link_libraries(${OMNIA_GFX_LIB} PUBLIC ${OMNIA_STD_LIB})
target_link_libraries(${TEST_TARGET} PUBLIC ${OMNIA_STD_LIB})
target_link_libraries(${TEST_TARGET} PUBLIC ${OMNIA_GFX_LIB})

if (APPLE)
	cmake_policy(SET CMP0167 OLD)
	find_package(Boost REQUIRED COMPONENTS regex)
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC Boost::regex)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDL2 REQUIRED sdl2)
	pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
	pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
	pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
	target_include_directories(${OMNIA_GFX_LIB} PUBLIC
					${SDL2_INCLUDE_DIRS}
					${SDL2_IMAGE_INCLUDE_DIRS}
					${SDL2_TTF_INCLUDE_DIRS}
					${SDL2_GFX_INCLUDE_DIRS}
	)
	target_link_directories(${OMNIA_GFX_LIB} PUBLIC
					${SDL2_LIBRARY_DIRS}
					${SDL2_IMAGE_LIBRARY_DIRS}
					${SDL2_TTF_LIBRARY_DIRS}
					${SDL2_GFX_LIBRARY_DIRS}
	)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC
					${SDL2_LIBRARIES}
					${SDL2_IMAGE_LIBRARIES}
					${SDL2_TTF_LIBRARIES}
					${SDL2_GFX_LIBRARIES}
	)
elseif (UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN',-rpath,/usr/lib,-rpath,/usr/local/lib")
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC tinfo boost_regex)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC xcb xcb-randr)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC SDL2main SDL2 SDL2_gfx SDL2_ttf SDL2_image)
endif ()
#-----------------------------------------------------------------------------------------
