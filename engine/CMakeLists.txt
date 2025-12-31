# CMakeLists.txt

file(GLOB_RECURSE CORE_HEADERS *.hpp)
file(GLOB_RECURSE CORE_SOURCES *.cpp)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/rlImGui.cpp
)

add_library(engine STATIC ${CORE_SOURCES} ${CORE_HEADERS} ${IMGUI_SOURCES})

target_link_libraries(engine
        PUBLIC
        raylib
        EnTT::EnTT
)

target_include_directories(engine
        PRIVATE
        # For internal engine files - they can use "components/..."
        ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC
        # For external consumers (game) - they use "components/..."
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/vendor
        ${CMAKE_SOURCE_DIR}/vendor/imgui
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
)