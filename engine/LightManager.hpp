#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#define MAX_LIGHTS 50 // Max dynamic lights supported by shader

namespace sage
{
    class Camera;
    struct Light;

    enum LightType
    {
        LIGHT_DIRECTIONAL = 0,
        LIGHT_POINT
    };

    class LightManager
    {
        entt::registry* registry;
        Camera* camera;
        Shader defaultShader{};
        std::vector<Shader> shaders;
        int lightsCount = 0;
        float gamma = 1.9;
        std::array<float, 4> ambient{};
        void updateShaderLights(Shader& _shader);
        void updateAmbientLight(Shader& _shader) const;

      public:
        void RemoveLight(entt::entity light);
        entt::entity CreateLight(
            int type,
            Vector3 position,
            Vector3 target,
            Color color,
            float intensity); // Create a light and get shader locations
        void LinkShaderToLights(Shader& _shader);
        void SetAmbientLight(float r, float g, float b, float a);
        void SetGamma(float g);
        void RefreshLights();
        void LinkRenderableToLight(entt::entity entity) const;
        void DrawDebugLights() const;
        void Update() const;
        explicit LightManager(entt::registry* _registry, Camera* _camera);
    };
} // namespace sage
