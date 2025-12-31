file(GLOB_RECURSE GAME_SOURCES main.cpp src/*.cpp utils/*.cpp)
file(GLOB_RECURSE GAME_HEADERS src/*.hpp utils/*.hpp)

add_executable(game ${GAME_SOURCES} ${GAME_HEADERS})

# Add both src and utils as include directories
target_include_directories(game
        PRIVATE
        src
        utils
)

target_link_libraries(game PRIVATE engine)

# Symbolic link for resources folder
set(source "${CMAKE_SOURCE_DIR}/resources")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/resources")
add_custom_command(
        TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)