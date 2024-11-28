//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ControllableActorSystem.hpp"

#include "AssetID.hpp"
#include "components/Collideable.hpp"
#include "components/ControllableActor.hpp"
#include "components/sgTransform.hpp"
#include "components/States.hpp"
#include "Cursor.hpp"
#include "EntityReflectionSignalRouter.hpp"
#include "GameData.hpp"
#include "PartySystem.hpp"
#include "ResourceManager.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "TextureTerrainOverlay.hpp"

#include <memory>

// TODO: Maybe combine this with PartySystem
namespace sage
{
    void ControllableActorSystem::Update() const
    {
        for (auto view = registry->view<ControllableActor, sgTransform, Collideable>(); auto entity : view)
        {
            auto& controllable = registry->get<ControllableActor>(entity);
            auto& trans = registry->get<sgTransform>(entity);
            auto& collideable = registry->get<Collideable>(entity);
            auto pos = trans.GetWorldPos();
            controllable.selectedIndicator->Update(pos);
        }
    }

    void ControllableActorSystem::SetSelectedActor(entt::entity id)
    {
        if (id == selectedActorId) return;
        if (selectedActorId != entt::null)
        {
            auto& old = registry->get<ControllableActor>(selectedActorId);
            old.selectedIndicator->SetShader(
                ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/base.fs"));
            old.selectedIndicator->SetHint(inactiveCol);
        }
        selectedActorId = id;

        auto& current = registry->get<ControllableActor>(selectedActorId);
        current.selectedIndicator->SetHint(activeCol);
        current.selectedIndicator->SetShader(
            ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/base.fs"));

        auto& controllable = registry->get<ControllableActor>(id);
        controllable.ReleaseAllHooks(gameData->reflectionSignalRouter.get());

        // Below forwards the cursor's events with the subscriber's entity ID injected into it (so we know which
        // entity is reacting to the click).
        controllable.AddHook(gameData->reflectionSignalRouter->CreateHook<entt::entity>(
            id, gameData->cursor->onFloorClick, controllable.onFloorClick));
        controllable.AddHook(gameData->reflectionSignalRouter->CreateHook<entt::entity>(
            id, gameData->cursor->onEnemyLeftClick, controllable.onEnemyLeftClick));
        controllable.AddHook(gameData->reflectionSignalRouter->CreateHook<entt::entity>(
            id, gameData->cursor->onEnemyRightClick, controllable.onEnemyRightClick));
        controllable.AddHook(gameData->reflectionSignalRouter->CreateHook<entt::entity>(
            id, gameData->cursor->onNPCClick, controllable.onNPCLeftClick));

        for (const auto group = gameData->partySystem->GetGroup(id); const auto& entity : group)
        {
            if (registry->any_of<PlayerState>(entity))
            {
                registry->erase<PlayerState>(entity);
            }
            if (registry->any_of<PartyMemberState>(entity))
            {
                registry->erase<PartyMemberState>(entity);
            }
        }
        registry->emplace<PlayerState>(id);
        for (const auto group = gameData->partySystem->GetGroup(id); const auto& entity : group)
        {
            if (entity != id)
            {
                registry->emplace<PartyMemberState>(entity);
            }
        }

        onSelectedActorChange.publish(id);
    }

    entt::entity ControllableActorSystem::GetSelectedActor() const
    {
        return selectedActorId;
    }

    void ControllableActorSystem::onComponentAdded(entt::entity addedEntity)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/particles/circle_03.png");
        auto& controllable = registry->get<ControllableActor>(addedEntity);
        controllable.selectedIndicator = std::make_unique<TextureTerrainOverlay>(
            registry,
            gameData->navigationGridSystem.get(),
            ResourceManager::GetInstance().TextureLoad("resources/textures/particles/circle_03.png"),
            inactiveCol,
            "resources/shaders/glsl330/base.fs");
        auto& trans = registry->get<sgTransform>(addedEntity);

        auto& collideable = registry->get<Collideable>(addedEntity);
        auto r = (collideable.localBoundingBox.max.x - collideable.localBoundingBox.min.x) * 0.5f;
        r += (collideable.localBoundingBox.max.z - collideable.localBoundingBox.min.z) * 0.5f;

        // TODO: This is currently not centered correctly
        controllable.selectedIndicator->Init(trans.GetWorldPos(), r);
        controllable.selectedIndicator->Enable(true);
    }

    void ControllableActorSystem::onComponentRemoved(entt::entity removedEntity)
    {
    }

    ControllableActorSystem::ControllableActorSystem(entt::registry* _registry, GameData* _gameData)
        : BaseSystem(_registry), gameData(_gameData)
    {
        registry->on_construct<ControllableActor>().connect<&ControllableActorSystem::onComponentAdded>(this);
        registry->on_destroy<ControllableActor>().connect<&ControllableActorSystem::onComponentRemoved>(this);
    }
} // namespace sage
