#Variables
#-----------------------------------------------------------------------------------------
set(PROJ_NAME KeyLight)
set(MAJOR_VER 0)
set(MINOR_VER 1)
#-----------------------------------------------------------------------------------------

#Setup
#-----------------------------------------------------------------------------------------
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
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
	${CMAKE_CURRENT_LIST_DIR}/vendor/midifile
)
list(APPEND SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/vendor/midifile/Binasc.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/vendor/midifile/MidiEvent.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/vendor/midifile/MidiEventList.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/vendor/midifile/MidiFile.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/vendor/midifile/MidiMessage.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/vendor/midifile/Options.cpp


	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/MidiParser.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Window.cpp
)
#-----------------------------------------------------------------------------------------

#Targets
#-----------------------------------------------------------------------------------------
set(MAIN_EXECUTABLE KeyLight)
add_executable(${MAIN_EXECUTABLE} ${SOURCE_FILES})
target_include_directories(${MAIN_EXECUTABLE} PUBLIC ${INCLUDE_DIRS})

target_compile_definitions(${MAIN_EXECUTABLE} PUBLIC  BUILD_NR=${BUILD_NUMBER} MAJ_V=${MAJOR_VER} MIN_V=${MINOR_VER} VERSION_STR="${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")

#TODO: Different flags for Release/Debug
add_compile_options(-O3 -m32 -MMD -MP -Wall -ggdb)
if (UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN',-rpath,/usr/lib"					)
	target_link_libraries(${MAIN_EXECUTABLE} xcb xcb-randr boost_regex)
endif (UNIX)
if (WIN32)
	target_link_libraries(${MAIN_EXECUTABLE} mingw32)
endif (WIN32)
target_link_libraries(${MAIN_EXECUTABLE} SDL2main SDL2)
#-----------------------------------------------------------------------------------------

#Linking 
#-----------------------------------------------------------------------------------------
target_link_libraries(${MAIN_EXECUTABLE} ostd)
target_link_libraries(${MAIN_EXECUTABLE} ogfx)
#-----------------------------------------------------------------------------------------

add_custom_command(TARGET ${MAIN_EXECUTABLE} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/extra/ $<TARGET_FILE_DIR:${MAIN_EXECUTABLE}>
	VERBATIM
)
