file(GLOB MAP_LOADER_HEADERS *.hpp)
file(GLOB MAP_LOADER_SOURCES *.cpp)

add_executable(respacker main.cpp ${MAP_LOADER_SOURCES} ${MAP_LOADER_HEADERS})
#target_include_directories(maploader PRIVATE vendor/assimp-src/include)
target_link_libraries(respacker
        PRIVATE
        core
)

# Symbolic link for resources folder
set(source "${CMAKE_SOURCE_DIR}/resources")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/resources")
add_custom_command(
        TARGET respacker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        DEPENDS ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)