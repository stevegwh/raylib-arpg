cmake_minimum_required(VERSION 3.26)
project(BG_Raylib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_EDITOR "Build the editor" OFF)
option(BUILD_RESPACKER "Build the resource packer" ON)
option(BUILD_GAME "Build the game" ON)

# Tell the engine where to find project-specific extension headers
# (e.g. project/CustomCollisionLayers.hpp). Must be set before loading Sage.
set(SAGE_PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR})
set(SAGE_BUILD_EDITOR ${BUILD_EDITOR} CACHE BOOL "Build the Sage editor" FORCE)

set(SAGE_SOURCE_DIR "" CACHE PATH "Override path to a local Sage checkout")
set(SAGE_SUBMODULE_DIR "${CMAKE_SOURCE_DIR}/external/sage")
if (SAGE_SOURCE_DIR)
    add_subdirectory("${SAGE_SOURCE_DIR}" ${CMAKE_BINARY_DIR}/_deps/sage-build)
elseif (EXISTS "${SAGE_SUBMODULE_DIR}/CMakeLists.txt")
    add_subdirectory("${SAGE_SUBMODULE_DIR}" ${CMAKE_BINARY_DIR}/_deps/sage-build)
else ()
    message(FATAL_ERROR
            "Sage submodule is missing. Run "
            "`git submodule update --init --recursive`, or configure with "
            "`-DSAGE_SOURCE_DIR=/path/to/sage`."
    )
endif ()

# Add subdirectories
add_subdirectory(game)

if (BUILD_RESPACKER)
    add_subdirectory(respacker)
endif ()
