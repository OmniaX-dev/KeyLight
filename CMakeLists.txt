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
 ${CMAKE_CURRENT_LIST_DIR}/src/SFMLWindow.cpp
 ${CMAKE_CURRENT_LIST_DIR}/src/Common.cpp
 ${CMAKE_CURRENT_LIST_DIR}/src/VirtualPiano.cpp
 ${CMAKE_CURRENT_LIST_DIR}/src/Renderer.cpp
 ${CMAKE_CURRENT_LIST_DIR}/src/Gui.cpp
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
target_compile_definitions(${MAIN_EXECUTABLE} PUBLIC BUILD_CONFIG="${CMAKE_BUILD_TYPE}" BUILD_NR=${BUILD_NUMBER} MAJ_V=${MAJOR_VER} MIN_V=${MINOR_VER} VERSION_STR="${MAJOR_VER}.${MINOR_VER}.${BUILD_NUMBER}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O3 -MMD -MP -Wall -ggdb)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
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
endif (WIN32)

if (UNIX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN',-rpath,/usr/lib,-rpath,/usr/local/lib")
    target_link_libraries(${MAIN_EXECUTABLE} xcb xcb-randr boost_regex)
endif (UNIX)

target_link_libraries(${MAIN_EXECUTABLE} SDL2main SDL2)
target_link_libraries(${MAIN_EXECUTABLE} sfml-system sfml-window sfml-graphics sfml-audio tgui)
target_link_libraries(${MAIN_EXECUTABLE} ostd)
# target_link_libraries(${MAIN_EXECUTABLE} ogfx)
#-----------------------------------------------------------------------------------------
