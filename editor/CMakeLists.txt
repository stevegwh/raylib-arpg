# editor/CMakeLists.txt

file(GLOB EDITOR_HEADERS *.hpp)
file(GLOB EDITOR_SOURCES *.cpp)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)

add_library(imgui_raylib STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
        ${IMGUI_DIR}/backends/rlImGui.cpp
)

target_include_directories(imgui_raylib
        PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/misc/cpp
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui_raylib
        PUBLIC
        raylib
)

add_executable(editor main.cpp ${EDITOR_SOURCES} ${EDITOR_HEADERS})

target_link_libraries(editor
        PRIVATE
        engine
        imgui_raylib
)

# Symbolic link for resources folder
set(source "${CMAKE_SOURCE_DIR}/resources")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/resources")
add_custom_command(
        TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)
