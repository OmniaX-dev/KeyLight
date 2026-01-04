#Variables
#-----------------------------------------------------------------------------------------
set(PROJECT_NAME KeyLight)
set(MAJOR_VER 0)
set(MINOR_VER 1)
#-----------------------------------------------------------------------------------------

#Setup
#-----------------------------------------------------------------------------------------
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.18)
project(${PROJECT_NAME} LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
file(STRINGS "./other/build.nr" BUILD_NUMBER)

if (APPLE)
	execute_process(
					COMMAND brew --prefix
					OUTPUT_VARIABLE HOMEBREW_PREFIX
					OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	set(CMAKE_INSTALL_RPATH "${HOMEBREW_PREFIX}/lib;/usr/local/lib")
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif(APPLE)
#-----------------------------------------------------------------------------------------

message(STATUS "Building ${PROJECT_NAME} ${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message(STATUS "Debug Configuration")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "Release Configuration")
endif()

#Sources
#-----------------------------------------------------------------------------------------
list(APPEND INCLUDE_DIRS
	${CMAKE_CURRENT_LIST_DIR}/src
)
list(APPEND SOURCE_FILES
	# ========================================== Main Code ==========================================
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Window.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/SFMLWindow.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Common.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VirtualPiano.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Renderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Gui.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ffmpeg_helper.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Particles.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VPianoResources.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VideoRenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VPianoData.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VirtualKeyboard.cpp
)
#-----------------------------------------------------------------------------------------



#Target
#-----------------------------------------------------------------------------------------
set(MAIN_EXECUTABLE KeyLight)
add_executable(${MAIN_EXECUTABLE} ${SOURCE_FILES})
if(WIN32)
	target_sources(${MAIN_EXECUTABLE} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/other/appIcon.rc")
endif()
target_include_directories(${MAIN_EXECUTABLE} PUBLIC ${INCLUDE_DIRS})
target_compile_definitions(${MAIN_EXECUTABLE} PUBLIC BUILD_NR=${BUILD_NUMBER} MAJ_V=${MAJOR_VER} MIN_V=${MINOR_VER} VERSION_STR="${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${MAIN_EXECUTABLE} PUBLIC BUILD_CONFIG_DEBUG)
	add_compile_options(-O3 -MMD -MP -Wall -ggdb)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(${MAIN_EXECUTABLE} PUBLIC BUILD_CONFIG_DEBUG)
	add_compile_options(-MMD -MP -Wall)
endif()

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
	set_target_properties(${MAIN_EXECUTABLE} PROPERTIES
								WIN32_EXECUTABLE TRUE
				)
endif()
#-----------------------------------------------------------------------------------------

#Linking
#-----------------------------------------------------------------------------------------
if (WIN32)
	target_link_libraries(${MAIN_EXECUTABLE} mingw32)
	target_link_libraries(${MAIN_EXECUTABLE} intl iconv)
	target_link_libraries(${MAIN_EXECUTABLE} ws2_32 shlwapi)
	target_link_libraries(${MAIN_EXECUTABLE} boost_filesystem-mt)
endif (WIN32)

if (APPLE)
	find_package(glm CONFIG REQUIRED)
	target_link_libraries(${MAIN_EXECUTABLE} glm::glm)
	find_path(GETTEXT_INCLUDE_DIR libintl.h
					PATHS
									/usr/local/include
									/opt/homebrew/include
									/usr/local/opt/gettext/include
									/opt/homebrew/opt/gettext/include
	)
	find_library(GETTEXT_LIBRARY intl
					PATHS
									/usr/local/lib
									/opt/homebrew/lib
									/usr/local/opt/gettext/lib
									/opt/homebrew/opt/gettext/lib
	)
	target_include_directories(${MAIN_EXECUTABLE} PUBLIC ${GETTEXT_INCLUDE_DIR})
	target_link_libraries(${MAIN_EXECUTABLE} ${GETTEXT_LIBRARY})
elseif (UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN',-rpath,/usr/lib,-rpath,/usr/local/lib")
	target_link_libraries(${MAIN_EXECUTABLE} xcb xcb-randr boost_regex)
endif ()

target_link_libraries(${MAIN_EXECUTABLE} sfml-system sfml-window sfml-graphics sfml-audio tgui)
target_link_libraries(${MAIN_EXECUTABLE} ostd)
#-----------------------------------------------------------------------------------------


# === Collect all translatable sources ===
file(GLOB_RECURSE TRANSLATABLE_SOURCES
				src/*.cpp
				src/*.h
)

# === Include gettext module ===
set(GETTEXT_SOURCES ${TRANSLATABLE_SOURCES})
set(GETTEXT_LANGUAGES en es it)
include(other/Gettext.cmake)

# Link your target to translations
add_dependencies(${MAIN_EXECUTABLE} translations)
