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
        Shader defaultShader{};
        std::vector<Shader> shaders;
        int lightsCount = 0;
        float gamma = 2.3;
        std::array<float, 4> ambient;
        void updateShaderLights(Shader& _shader);
        void updateAmbientLight(Shader& _shader) const;

      public:
        void CreateLight(
            Shader& _shader,
            int type,
            Vector3 position,
            Vector3 target,
            Color color); // Create a light and get shader locations
        void LinkShaderToLights(Shader& _shader);
        void SetAmbientLight(float r, float g, float b, float a);
        void SetGamma(float g);
        void RefreshLights();
        void LinkRenderableToLight(entt::entity entity) const;
        void DrawDebugLights() const;
        void Update() const;
        explicit LightSubSystem(entt::registry* _registry, Camera* _camera);
    };
} // namespace sage
