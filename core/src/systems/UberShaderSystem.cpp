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
        uber.emissiveLoc = emissiveLoc;
        auto& renderable = registry->get<Renderable>(entity);

        auto& materials = renderable.GetModel()->rlmodel.materials;
        for (int i = 0; i < renderable.GetModel()->rlmodel.materialCount; ++i)
        {
            // auto col = renderable.GetModel()->rlmodel.materials[i].maps[MATERIAL_MAP_EMISSION].color;
            auto emissiveTex = renderable.GetModel()->rlmodel.materials[i].maps[MATERIAL_MAP_EMISSION].texture.id;
            //            if (emissiveTex > 1 || (col.r != 0 || col.g != 0 || col.b != 0))
            if (emissiveTex > 1)
            {
                // Means that if one material is emissive then all will be set as it.
                uber.SetFlag(i, UberShaderComponent::Flags::Emissive);
                break;
            }
        }

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

        shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "emissionMap");
        shader.locs[SHADER_LOC_COLOR_AMBIENT] =
            GetShaderLocation(shader, "emissionCol");         // stealing ambient color slot for emission
        gameData->lightSubSystem->LinkShaderToLights(shader); // Links shader to light data
        litLoc = GetShaderLocation(shader, "lit");
        skinnedLoc = GetShaderLocation(shader, "skinned");
        emissiveLoc = GetShaderLocation(shader, "emission");
    }
} // namespace sage