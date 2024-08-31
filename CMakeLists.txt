#Variables
#-----------------------------------------------------------------------------------------
set(PROJ_NAME omnia-framework)
set(MAJOR_VER 0)
set(MINOR_VER 1)
set(OMNIA_STD_LIB ostd)
set(OMNIA_GFX_LIB ogfx)
set(TEST_TARGET ostd_test)
set(RUN_CONFIG O_F_DEBUG)
#-----------------------------------------------------------------------------------------

#Setup
#-----------------------------------------------------------------------------------------
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
cmake_minimum_required(VERSION 3.18)
project(${PROJ_NAME} LANGUAGES C CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
file(STRINGS "./build.nr" BUILD_NUMBER)
#-----------------------------------------------------------------------------------------

message("** Building ${PROJ_NAME} ${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")

#Sources
#-----------------------------------------------------------------------------------------
list(APPEND INCLUDE_DIRS
	${CMAKE_CURRENT_LIST_DIR}/src
	${CMAKE_CURRENT_LIST_DIR}/src/ostd
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/vendor
)
list(APPEND OSTD_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/BaseObject.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Color.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Console.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Errors.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/File.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/FileSystem.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Geometry.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Keyboard.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Logger.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Logic.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/OutputHandlers.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/PathFinder.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/QuadTree.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/ShuntingYard.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Signals.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Random.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Serial.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/SineWave.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/StringEditor.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/String.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/TextStyleParser.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Utils.cpp
)
list(APPEND OGFX_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/FontUtils.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/WindowBase.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/BasicRenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/RawTextInput.cpp
)
list(APPEND TEST_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/test.cpp
)
#-----------------------------------------------------------------------------------------

#Targets
#-----------------------------------------------------------------------------------------
add_library(${OMNIA_STD_LIB} SHARED ${OSTD_SOURCE_FILES})
add_library(${OMNIA_GFX_LIB} SHARED ${OGFX_SOURCE_FILES})
add_executable(${TEST_TARGET} ${TEST_SOURCE_FILES})

target_compile_definitions(${OMNIA_STD_LIB} PUBLIC ${RUN_CONFIG} BUILD_NR=${BUILD_NUMBER} MAJ_V=${MAJOR_VER} MIN_V=${MINOR_VER} VERSION_STR="${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
target_compile_definitions(${OMNIA_GFX_LIB} PUBLIC ${RUN_CONFIG} BUILD_NR=${BUILD_NUMBER} MAJ_V=${MAJOR_VER} MIN_V=${MINOR_VER} VERSION_STR="${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
target_include_directories(${OMNIA_STD_LIB} PUBLIC ${INCLUDE_DIRS})
target_include_directories(${OMNIA_GFX_LIB} PUBLIC ${INCLUDE_DIRS})
target_include_directories(${TEST_TARGET} PUBLIC ${INCLUDE_DIRS})

target_link_libraries(${OMNIA_GFX_LIB} PUBLIC ${OMNIA_STD_LIB})
target_link_libraries(${TEST_TARGET} PUBLIC ${OMNIA_STD_LIB})
target_link_libraries(${TEST_TARGET} PUBLIC ${OMNIA_GFX_LIB})

#TODO: Different flags for Release/Debug
add_compile_options(-O3 -m32 -MMD -MP -Wall -ggdb -fsanitize=address)
if (UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC tinfo boost_regex)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC xcb xcb-randr)
endif (UNIX)
target_link_libraries(${OMNIA_GFX_LIB} PUBLIC SDL2main SDL2 SDL2_gfx SDL2_ttf)
#-----------------------------------------------------------------------------------------

#Finalize
#-----------------------------------------------------------------------------------------
add_custom_command(TARGET ${OMNIA_STD_LIB} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/extra/ $<TARGET_FILE_DIR:${OMNIA_STD_LIB}>
	VERBATIM
)
add_custom_target(FinalMessage ALL ${CMAKE_COMMAND} -E cmake_echo_color --cyan "[100%] Built ${PROJ_NAME} ${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
add_dependencies(FinalMessage ${OMNIA_GFX_LIB})
#-----------------------------------------------------------------------------------------
