cmake_minimum_required(VERSION 3.26)
project(BG_Raylib)

set(CMAKE_CXX_STANDARD 20)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -O3 -Xclang -fopenmp")
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

# Dependencies
include(FetchContent)
FetchContent_Declare(
        entt
        GIT_REPOSITORY "https://github.com/skypjack/entt.git"
        GIT_TAG "v3.13.2"
)
FetchContent_MakeAvailable(entt)
set(GRAPHICS GRAPHICS_API_OPENGL_43)
add_subdirectory(vendor/raylib)
include_directories(vendor)


# Option to enable/disable building the editor
option(BUILD_EDITOR "Build the editor" ON)

# Add the core subdirectory
add_subdirectory(core)

# Conditionally add the editor subdirectory
if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()

# Add the game executable target
add_executable(game core/src/main.cpp)
target_link_libraries(game PRIVATE core)

# Symbolic link for resources folder (game executable)
set(source "${CMAKE_SOURCE_DIR}/resources")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/resources")
add_custom_command(
        TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        DEPENDS ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)