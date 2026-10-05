#Variables
#-----------------------------------------------------------------------------------------
set(PROJ_NAME omnia-framework)
set(MAJOR_VER 0)
set(MINOR_VER 4)
#-----------------------------------------------------------------------------------------

#Setup
#-----------------------------------------------------------------------------------------
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.18)
project(${PROJ_NAME} LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
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
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/data/Object.cpp
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
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/io/Stylesheet.cpp

	# math
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/ShuntingYard.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/Random.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/SineWave.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/math/Spline.cpp

	# string
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/string/String.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/string/TextStyleParser.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/string/utf8.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/string/TextBuffer.cpp

	# utils
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Date.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Logic.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Hash.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/PathFinder.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/QuadTree.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Signals.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ostd/utils/Time.cpp
)
list(APPEND OGFX_SOURCE_FILES
	# gui
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/RawTextInput.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/Window.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/WindowOutputHandler.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/WidgetManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/ContextMenu.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/MenuBar.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/NativeMenu.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/ToolBar.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/Layout.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/FocusManager.cpp

	# gui/widgets
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/Widget.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/Containers.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/Label.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/RootWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/CheckBox.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/Scrollbar.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/Button.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/RadioButton.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/ProgressBar.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/Slider.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/ComboBox.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/TreeView.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/DetailList.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/LineGraph.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/Text.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/ColorPicker.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/widgets/ColorPickerSkins.cpp

	# render
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/render/BasicRenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/render/PixelRenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/render/FontGlyphAtlas.cpp

	# resources
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/resources/Image.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/resources/UbuntuTTF.cpp

	# utils
	${CMAKE_CURRENT_LIST_DIR}/src/ogfx/utils/Animation.cpp
)
if (APPLE)
	# Objective-C++ bridge for the native macOS global menu backend (NativeMenu.hpp) - compiled
	# with ARC so CocoaMenuBackend.mm can use modern Cocoa ownership without manual retain/release.
	list(APPEND OGFX_SOURCE_FILES
		${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/macos/CocoaMenuBackend.mm
	)
	set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/macos/CocoaMenuBackend.mm PROPERTIES COMPILE_FLAGS "-fobjc-arc")
endif ()
if (UNIX AND NOT APPLE)
	# Native Plasma global-menu backend (NativeMenu.hpp): DBusMenu over raw libdbus-1, associated
	# with the window via the X11 AppMenu registrar or (Wayland) KWin's org_kde_kwin_appmenu
	# protocol extension - see src/ogfx/gui/menu/linux/.
	list(APPEND OGFX_SOURCE_FILES
		${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/NativeWindowHandle.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/linux/DBusMenuServer.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/linux/X11AppMenuRegistrar.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/linux/WaylandAppMenuBinder.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/linux/PlasmaMenuBackend.cpp
	)

	# appmenu.xml (vendored from Qt6's wayland protocols, LGPL-2.1-or-later, matching its own
	# license header - standard practice for Wayland protocol consumers) -> client bindings,
	# generated at build time into the binary dir rather than committing generated code.
	set(APPMENU_PROTOCOL_XML ${CMAKE_CURRENT_LIST_DIR}/src/ogfx/gui/menu/linux/protocols/appmenu.xml)
	set(APPMENU_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
	set(APPMENU_GENERATED_HEADER ${APPMENU_GENERATED_DIR}/appmenu-client-protocol.h)
	set(APPMENU_GENERATED_CODE ${APPMENU_GENERATED_DIR}/appmenu-client-protocol.c)
	find_program(WAYLAND_SCANNER_EXECUTABLE wayland-scanner REQUIRED)
	file(MAKE_DIRECTORY ${APPMENU_GENERATED_DIR})
	add_custom_command(
		OUTPUT ${APPMENU_GENERATED_HEADER}
		COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header ${APPMENU_PROTOCOL_XML} ${APPMENU_GENERATED_HEADER}
		DEPENDS ${APPMENU_PROTOCOL_XML}
	)
	add_custom_command(
		OUTPUT ${APPMENU_GENERATED_CODE}
		COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code ${APPMENU_PROTOCOL_XML} ${APPMENU_GENERATED_CODE}
		DEPENDS ${APPMENU_PROTOCOL_XML}
	)
	list(APPEND OGFX_SOURCE_FILES ${APPMENU_GENERATED_CODE} ${APPMENU_GENERATED_HEADER})
	set_source_files_properties(${APPMENU_GENERATED_HEADER} PROPERTIES GENERATED TRUE)
	set_source_files_properties(${APPMENU_GENERATED_CODE} PROPERTIES GENERATED TRUE)
endif ()
list(APPEND TEST_SOURCE_FILES
	# ${CMAKE_CURRENT_LIST_DIR}/src/test/GraphicsWindowTest.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/test/GuiTest.cpp
	# ${CMAKE_CURRENT_LIST_DIR}/src/test/TextBufferUnitTest.cpp
	# ${CMAKE_CURRENT_LIST_DIR}/src/test/LogFileOutputHandlerTest.cpp
	# ${CMAKE_CURRENT_LIST_DIR}/src/test/ObjectSignalExamples.cpp
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
	add_compile_options(-MMD -MP -Wall -latomic)
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
	pkg_check_modules(SDL3_IMAGE REQUIRED sdl3-image)
	pkg_check_modules(SDL3_TTF REQUIRED sdl3-ttf)
	target_include_directories(${OMNIA_GFX_LIB} PUBLIC
					${SDL3_INCLUDE_DIRS}
					${SDL3_IMAGE_INCLUDE_DIRS}
					${SDL3_TTF_INCLUDE_DIRS}
	)
	target_link_directories(${OMNIA_GFX_LIB} PUBLIC
					${SDL3_LIBRARY_DIRS}
					${SDL3_IMAGE_LIBRARY_DIRS}
					${SDL3_TTF_LIBRARY_DIRS}
	)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC
					${SDL3_LIBRARIES}
					${SDL3_IMAGE_LIBRARIES}
					${SDL3_TTF_LIBRARIES}
	)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC "-framework Cocoa")
elseif (UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN',-rpath,/usr/lib,-rpath,/usr/local/lib")
	target_link_libraries(${OMNIA_STD_LIB} PUBLIC tinfo boost_regex)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC xcb xcb-randr)
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC SDL3 SDL3_ttf SDL3_image)

	# dbus-1's headers live under a versioned subdirectory (#include <dbus/dbus.h> needs both
	# of these) - pkg-config is only used to resolve that non-obvious include path; linked by
	# plain name below like everything else in this branch.
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(DBUS REQUIRED dbus-1)
	target_include_directories(${OMNIA_GFX_LIB} PUBLIC ${DBUS_INCLUDE_DIRS} ${APPMENU_GENERATED_DIR})
	target_link_libraries(${OMNIA_GFX_LIB} PUBLIC dbus-1 wayland-client)
endif ()
#-----------------------------------------------------------------------------------------
