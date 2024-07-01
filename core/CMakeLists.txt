# core/CMakeLists.txt

file(GLOB_RECURSE CORE_HEADERS src/*.hpp utils/*.hpp)
file(GLOB_RECURSE CORE_SOURCES src/*.cpp  utils/*.cpp)

add_library(core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

find_package(OpenMP REQUIRED)
target_link_libraries(core
        PUBLIC
        raylib
        EnTT::EnTT
        OpenMP::OpenMP_CXX
)

target_include_directories(core
        PUBLIC
        src
        utils
        ${CMAKE_SOURCE_DIR}/vendor
)