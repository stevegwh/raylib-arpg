# game/CMakeLists.txt

file(GLOB_RECURSE GAME_SOURCES src/*.cpp utils/*.cpp)
file(GLOB_RECURSE GAME_HEADERS src/*.hpp utils/*.hpp)

# Create a library with the game code
add_library(gamelib STATIC ${GAME_SOURCES} ${GAME_HEADERS})

target_include_directories(gamelib
        PUBLIC  # Changed to PUBLIC so respacker can use these
        src
        utils
)

target_link_libraries(gamelib PUBLIC engine)

# Create the game executable with just main.cpp
add_executable(game main.cpp)
target_link_libraries(game PRIVATE gamelib)

# Symbolic link for resources folder
set(source "${CMAKE_SOURCE_DIR}/resources")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/resources")
add_custom_command(
        TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)