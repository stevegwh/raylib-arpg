//
// Created by steve on 04/01/2025.
//

#include "SpatialAudioSystem.hpp"

#include "EngineSystems.hpp"

#include "components/sgTransform.hpp"
#include "components/SpatialAudioComponent.hpp"

namespace sage
{

    void SpatialAudioSystem::Update() const
    {
        for (const auto view = registry->view<SpatialAudioComponent, sgTransform>(); const auto& entity : view)
        {
            auto& spatial = registry->get<SpatialAudioComponent>(entity);
        }
    }

    SpatialAudioSystem::SpatialAudioSystem(entt::registry* _registry, EngineSystems* _sys)
        : registry(_registry), sys(_sys)
    {
    }
} // namespace sage