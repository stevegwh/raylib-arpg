#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

#define MAX_LIGHTS 25 // Max dynamic lights supported by shader

namespace sage
{
    class Camera;
    struct Light;

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
        int ambientLoc;
        int lightsCountLoc;
        int lightsCount = 0;

        Light createLight(
            int type,
            Vector3 position,
            Vector3 target,
            Color color,
            Shader shader); // Create a light and get shader locations
      public:
        void LinkRenderableToLight(entt::entity entity) const;
        void UpdateAmbientLight(float r, float g, float b, float a) const;
        void RefreshLights();
        void DrawDebugLights() const;
        void Update() const;
        explicit LightSubSystem(entt::registry* _registry, Camera* _camera);
    };
} // namespace sage
