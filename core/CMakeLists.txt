# core/CMakeLists.txt

file(GLOB_RECURSE CORE_HEADERS src/*.hpp utils/*.hpp)
file(GLOB_RECURSE CORE_SOURCES src/*.cpp utils/*.cpp)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/rlImGui.cpp
)

add_library(core STATIC ${CORE_SOURCES} ${CORE_HEADERS} ${IMGUI_SOURCES})

#find_package(OpenMP REQUIRED)
target_link_libraries(core
        PUBLIC
        raylib
        EnTT::EnTT
        #OpenMP::OpenMP_CXX
)

target_include_directories(core
        PUBLIC
        src
        utils
        ${CMAKE_SOURCE_DIR}/vendor
        ${CMAKE_SOURCE_DIR}/vendor/imgui
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
)