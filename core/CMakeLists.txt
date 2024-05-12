# core/CMakeLists.txt

file(GLOB CORE_HEADERS src/*.hpp src/components/*.hpp src/systems/*.hpp src/systems/dialogue/*.hpp src/scenes/*.hpp utils/*.hpp)
file(GLOB CORE_SOURCES src/*.cpp src/components/*.cpp src/systems/*.cpp src/systems/dialogue/*.cpp src/scenes/*.cpp utils/*.cpp)

add_library(core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_link_libraries(core
        PUBLIC
        raylib
        EnTT::EnTT
)

target_include_directories(core
        PUBLIC
        src
        utils
        ${CMAKE_SOURCE_DIR}/vendor
)