cmake_minimum_required(VERSION 3.26)
project(BG_Raylib)

set(CMAKE_CXX_STANDARD 20)

# Dependencies
include(FetchContent)
FetchContent_Declare(
        entt
        GIT_REPOSITORY "https://github.com/skypjack/entt.git"
        GIT_TAG "v3.13.2"
)
FetchContent_MakeAvailable(entt)

set(PLATFORM "Desktop" CACHE STRING "" FORCE)
add_subdirectory(vendor/raylib)
include_directories(vendor)
include_directories(vendor/magic_enum)
include_directories(vendor/imgui)
include_directories(vendor/imgui/backends)

# Options
option(BUILD_EDITOR "Build the editor" OFF)
option(BUILD_RESPACKER "Build the resource packer" ON)
option(BUILD_GAME "Build the game" ON)

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(game)

if (BUILD_EDITOR)
    add_subdirectory(editor)
endif()

if (BUILD_RESPACKER)
    add_subdirectory(respacker)
endif()