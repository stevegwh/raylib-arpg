cmake_minimum_required(VERSION 3.26)
project(BG_Raylib)

set(CMAKE_CXX_STANDARD 20)

# Dependencies
set(RAYLIB_VERSION 5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES ON CACHE BOOL "" FORCE) # don't build the supplied examples
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()

# Json
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

include_directories(../r3d-test/includes)

add_executable(BG_Raylib
        main.cpp
        GameManager.cpp
        GameManager.hpp
        Camera.cpp
        Camera.hpp
        Renderable.hpp
        Material.hpp
        UserInput.cpp
        UserInput.hpp
        CollisionSystem.cpp
        CollisionSystem.hpp
        Collideable.hpp
        Event.cpp
        Event.hpp
        raygui.h
        Entity.hpp
        Transform.hpp
        RenderSystem.cpp
        RenderSystem.hpp
        Component.hpp
        TransformSystem.cpp
        TransformSystem.hpp
        Entity.cpp
        BaseSystem.hpp
        Registry.hpp
        Editor.cpp
        Editor.hpp
        WorldObject.hpp
        WorldSystem.cpp
        WorldSystem.hpp
        NavigationGridSystem.cpp
        NavigationGridSystem.hpp
        NavigationGridSquare.hpp
        ActorMovementSystem.cpp
        ActorMovementSystem.hpp
        Actor.hpp
        GameObjectFactory.cpp
        GameObjectFactory.hpp
        Serializer.cpp
        Serializer.hpp
        Scene.hpp
        Game.cpp
        Game.hpp
        ECSManager.cpp
        ECSManager.hpp
        EventManager.hpp
        EventManager.cpp
        EventCallback.hpp
        AnimationSystem.cpp
        AnimationSystem.hpp
        Animation.hpp
        LightSubSystem.cpp
        LightSubSystem.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib nlohmann_json::nlohmann_json)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
#if (APPLE)
#    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
#    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
#    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
#endif()


set(source "${CMAKE_SOURCE_DIR}/resources")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/resources")
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        DEPENDS ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)