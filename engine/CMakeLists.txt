# CMakeLists.txt

# Path the consuming project provides for engine extension headers
# (e.g. project/CustomCollisionLayers.hpp). When unset, falls back to the
# engine's stub directory so the engine builds standalone.
if(NOT DEFINED SAGE_PROJECT_INCLUDE_DIR)
    set(SAGE_PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/defaults)
endif()

file(GLOB_RECURSE CORE_HEADERS *.hpp)
file(GLOB_RECURSE CORE_SOURCES *.cpp)

# Exclude the standalone-build stub from the glob — it shadows the real
# project header at compile time only via the include path.
list(FILTER CORE_HEADERS EXCLUDE REGEX "/defaults/")

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
        # Resolves project/CustomCollisionLayers.hpp (and any future project
        # extension headers). Defaults to engine/defaults/ for standalone builds.
        ${SAGE_PROJECT_INCLUDE_DIR}
)