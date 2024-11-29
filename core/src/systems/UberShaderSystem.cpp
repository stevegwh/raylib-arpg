//
// Created by steve on 21/11/2024.
//

#include "UberShaderSystem.hpp"
#include "components/Renderable.hpp"
#include "components/UberShaderComponent.hpp"
#include "GameData.hpp"
#include "LightManager.hpp"
#include "ResourceManager.hpp"

namespace sage
{

    void UberShaderSystem::onComponentAdded(entt::entity entity)
    {
        auto& uber = registry->get<UberShaderComponent>(entity);
        uber.shader = shader;
        uber.litLoc = litLoc;
        uber.skinnedLoc = skinnedLoc;
        auto& renderable = registry->get<Renderable>(entity);
        for (int i = 0; i < renderable.GetModel()->GetMaterialCount(); ++i)
        {
            renderable.GetModel()->SetShader(shader, i);
        }
    }

    void UberShaderSystem::onComponentRemoved(entt::entity entity)
    {
    }

    UberShaderSystem::UberShaderSystem(entt::registry* _registry, sage::GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        registry->on_construct<UberShaderComponent>().connect<&UberShaderSystem::onComponentAdded>(this);
        registry->on_destroy<UberShaderComponent>().connect<&UberShaderSystem::onComponentRemoved>(this);

        shader = ResourceManager::GetInstance().ShaderLoad(
            "resources/shaders/custom/ubershader.vs", "resources/shaders/custom/ubershader.fs");
        gameData->lightSubSystem->LinkShaderToLights(shader); // Links shader to light data
        litLoc = GetShaderLocation(shader, "lit");
        skinnedLoc = GetShaderLocation(shader, "skinned");
    }
} // namespace sage