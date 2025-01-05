//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ControllableActorSystem.hpp"

#include "common_types.hpp"
#include "components/Collideable.hpp"
#include "components/ControllableActor.hpp"
#include "components/sgTransform.hpp"
#include "components/States.hpp"
#include "Cursor.hpp"
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
        for (const auto view = registry->view<ControllableActor, sgTransform, Collideable>();
             const auto entity : view)
        {
            const auto& controllable = registry->get<ControllableActor>(entity);
            const auto& trans = registry->get<sgTransform>(entity);
            const auto pos = trans.GetWorldPos();
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

            // Stop forwarding cursor clicks to this actor
            old.cursorOnFloorClickCnx.UnSubscribe();
            old.cursorOnEnemyLeftClickCnx.UnSubscribe();
            old.cursorOnEnemyRightClickCnx.UnSubscribe();
            old.cursorOnNPCLeftClickCnx.UnSubscribe();
        }
        selectedActorId = id;

        auto& current = registry->get<ControllableActor>(selectedActorId);
        current.selectedIndicator->SetHint(activeCol);
        current.selectedIndicator->SetShader(
            ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/base.fs"));

        // Forward cursor clicks to this actor's controllable component's events
        current.cursorOnFloorClickCnx =
            gameData->cursor->onFloorClick->Subscribe([this](const entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(selectedActorId);
                c.onFloorClick->Publish(selectedActorId, clickedEntity);
            });
        current.cursorOnEnemyLeftClickCnx =
            gameData->cursor->onEnemyLeftClick->Subscribe([this](const entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(selectedActorId);
                c.onEnemyLeftClick->Publish(selectedActorId, clickedEntity);
            });
        current.cursorOnEnemyRightClickCnx =
            gameData->cursor->onEnemyRightClick->Subscribe([this](const entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(selectedActorId);
                c.onEnemyRightClick->Publish(selectedActorId, clickedEntity);
            });
        current.cursorOnNPCLeftClickCnx =
            gameData->cursor->onNPCClick->Subscribe([this](const entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(selectedActorId);
                c.onNPCLeftClick->Publish(selectedActorId, clickedEntity);
            });

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

        onSelectedActorChange->Publish(id);
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
        : BaseSystem(_registry),
          gameData(_gameData),
          onSelectedActorChange(std::make_unique<Event<entt::entity>>())
    {
        registry->on_construct<ControllableActor>().connect<&ControllableActorSystem::onComponentAdded>(this);
        registry->on_destroy<ControllableActor>().connect<&ControllableActorSystem::onComponentRemoved>(this);
    }
} // namespace sage
