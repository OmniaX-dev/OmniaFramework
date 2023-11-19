#Setup
#-----------------------------------------------------------------------------------------
set(CMAKE_CXX_COMPILER "clang++")
cmake_minimum_required(VERSION 3.18)
project(${PROJ_NAME} LANGUAGES C CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
file(STRINGS "./tools/build.nr" BUILD_NUMBER)
#-----------------------------------------------------------------------------------------



#Variables
#-----------------------------------------------------------------------------------------
set(PROJ_NAME omnia-framework)
set(MAJOR_VER 0)
set(MINOR_VER 1)
set(OMNIA_STD_LIB ostd)
set(OMNIA_GFX_LIB ogfx)
set(TEST_TARGET framework-test)
set(RUN_CONFIG O_F_DEBUG)

message("** Building ${PROJ_NAME} ${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
#-----------------------------------------------------------------------------------------



#Sources
#-----------------------------------------------------------------------------------------
list(APPEND OSTD_VENDOR_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/vendor/clip/clip.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/vendor/clip/image.cpp
)
list(APPEND OGFX_VENDOR_SOURCES	
	${CMAKE_CURRENT_LIST_DIR}/src/vendor/GLAD/glad.c
)

list(APPEND INCLUDE_DIRS
	${CMAKE_CURRENT_LIST_DIR}/src
	${CMAKE_CURRENT_LIST_DIR}/src/ostd
	${CMAKE_CURRENT_LIST_DIR}/src/vendor
	${CMAKE_CURRENT_LIST_DIR}/src/vendor/GLAD
)

list(APPEND TEST_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/test/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/test/TestApp2D.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/test/TestApp3D.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/test/TetrisApp.cpp
)
list(APPEND OSH_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/osh/commandShell.cpp
)
list(APPEND OXGE_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/oxge/iso/Engine.cpp
)
list(APPEND OGFX_SOURCE_FILES	
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/BitmapFont.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/BlendRenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Errors.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Camera.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/GraphicsApplication.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/GLBuffers.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Mesh.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/OX3DLoader.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Renderer2D.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/RenderCommands.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/RenderCore.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/RenderTarget.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/ResourceManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/RTData.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Shader.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Texture.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Window.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/Widgets.cpp

	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/DebugTools.cpp
)
list(APPEND OSTD_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/BaseObject.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Color.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Console.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/DataFile.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Errors.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/File.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/FileSystem.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Geometry.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Logger.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Logic.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/OutputHandlers.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/PathFinder.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/QuadTree.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/ShuntingYard.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Signals.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Random.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/RichString.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Serial.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/SineWave.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/StringEditor.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/Utils.cpp
)

if (WIN32)
	list(APPEND OSTD_VENDOR_SOURCES
		${CMAKE_CURRENT_LIST_DIR}/src/vendor/clip/clip_win.cpp
  )
endif (WIN32)
if (UNIX)
	list(APPEND OSTD_VENDOR_SOURCES
		${CMAKE_CURRENT_LIST_DIR}/src/vendor/clip/clip_x11.cpp
  )
endif (UNIX)

list(APPEND OSTD_SOURCE_FILES ${OSTD_VENDOR_SOURCES})
list(APPEND OGFX_SOURCE_FILES ${OGFX_VENDOR_SOURCES})
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

#TODO: Different flags for Release/Debug
add_compile_options(-O3 -m32 -MMD -MP -Wall -ggdb -fsanitize=address)
if (UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC X11 GL)
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC tinfo)
endif (UNIX)

target_link_libraries(${OMNIA_GFX_LIB} PUBLIC sfml-system sfml-window sfml-graphics)
#-----------------------------------------------------------------------------------------



#Linking Targets
#-----------------------------------------------------------------------------------------
target_link_libraries(${OMNIA_GFX_LIB} PUBLIC ${OMNIA_STD_LIB})

target_link_libraries(${TEST_TARGET} PUBLIC ${OMNIA_STD_LIB})
target_link_libraries(${TEST_TARGET} PUBLIC ${OMNIA_GFX_LIB})
#-----------------------------------------------------------------------------------------



#BuildNumber Target
#-----------------------------------------------------------------------------------------
if (WIN32)
	add_custom_command ( OUTPUT ./tools/build.nr
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/inc_bnr.exe
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/
	)
endif (WIN32)
if (UNIX)
	add_custom_command ( OUTPUT ./tools/build.nr
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/inc_bnr
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/
	)
endif (UNIX)

add_custom_target(
  IncBnr ALL
  DEPENDS ./tools/build.nr
)
#-----------------------------------------------------------------------------------------



#Install
#-----------------------------------------------------------------------------------------
#TODO: Change TEST_TARGET to last lib built, for release config
add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/extra/ $<TARGET_FILE_DIR:${TEST_TARGET}>
	VERBATIM
)

set(CMAKE_INSTALL_MESSAGE NEVER)
install(TARGETS ${OMNIA_STD_LIB} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/Build/bin)
install(TARGETS ${OMNIA_GFX_LIB} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/Build/bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/licences DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/Build)

add_custom_target(FinalMessage ALL ${CMAKE_COMMAND} -E cmake_echo_color --cyan "[100%] Built ${PROJ_NAME} ${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
add_dependencies(FinalMessage ${TEST_TARGET})
#-----------------------------------------------------------------------------------------
