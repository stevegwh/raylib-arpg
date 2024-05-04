# editor/CMakeLists.txt

file(GLOB EDITOR_HEADERS *.hpp)
file(GLOB EDITOR_SOURCES *.cpp)

add_executable(editor main.cpp ${EDITOR_SOURCES} ${EDITOR_HEADERS})

target_link_libraries(editor
        PRIVATE
        core
)

# Symbolic link for resources folder
set(source "${CMAKE_SOURCE_DIR}/resources")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/resources")
add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        DEPENDS ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)