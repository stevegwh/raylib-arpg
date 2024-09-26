/**********************************************************************************************
 *
 *   raylib.lights - Some useful functions to deal with lights data
 *
 *   CONFIGURATION:
 *
 *   #define RLIGHTS_IMPLEMENTATION
 *       Generates the implementation of the library into the included file.
 *       If not defined, the library is in header only mode and can be included in other headers
 *       or source files without problems. But only ONE file should hold the implementation.
 *
 *   LICENSE: zlib/libpng
 *
 *   Copyright (c) 2017-2023 Victor Fisac (@victorfisac) and Ramon Santamaria (@raysan5)
 *
 *   This software is provided "as-is", without any express or implied warranty. In no event
 *   will the authors be held liable for any damages arising from the use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose, including commercial
 *   applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not claim that you
 *     wrote the original software. If you use this software in a product, an acknowledgment
 *     in the product documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *     as being the original software.
 *
 *     3. This notice may not be removed or altered from any source distribution.
 *
 **********************************************************************************************/

#pragma once

#include "components/Light.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

#define MAX_LIGHTS 11 // Max dynamic lights supported by shader

namespace sage
{
    class Camera;

    enum LightType
    {
        LIGHT_DIRECTIONAL = 0,
        LIGHT_POINT
    };

    class LightSubSystem
    {
        entt::registry* registry;
        Camera* camera;
        Shader shader{};
        int lightsCount = 0;
        // std::vector<Light> lights{};

        Light createLight(
            int type,
            Vector3 position,
            Vector3 target,
            Color color,
            Shader shader); // Create a light and get shader locations
      public:
        // template <typename Archive>
        // void serialize(Archive& archive)
        // {
        //     archive(lights);
        // }
        void AddLight(Vector3 pos, Color col);
        void LinkRenderableToLight(entt::entity entity) const;
        void DrawDebugLights() const;
        void Update() const;
        void LinkAllLights();
        explicit LightSubSystem(entt::registry* _registry, Camera* _camera);
    };
} // namespace sage
