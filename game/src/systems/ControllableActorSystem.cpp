//
// Created by Steve Wheeler on 29/02/2024.
//

#include "systems/ControllableActorSystem.hpp"

#include "components/ControllableActor.hpp"
#include "components/States.hpp"
#include "PartySystem.hpp"
#include "Systems.hpp"

#include "engine/components/BaseStateComponent.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/systems/ActorMovementSystem.hpp"
#include "engine/TextureTerrainOverlay.hpp"

#include <memory>

// TODO: Maybe combine this with PartySystem
namespace lq
{
    void ControllableActorSystem::Update() const
    {
        for (const auto view = registry->view<ControllableActor, sage::sgTransform, sage::Collideable>();
             const auto entity : view)
        {
            const auto& controllable = registry->get<ControllableActor>(entity);
            const auto& trans = registry->get<sage::sgTransform>(entity);
            const auto pos = trans.GetWorldPos();
            controllable.selectedIndicator->Update(pos);
        }
    }

    void ControllableActorSystem::setSelectedActor(entt::entity oldEntity, entt::entity newEntity) const
    {
        if (oldEntity != entt::null)
        {
            auto& old = registry->get<ControllableActor>(oldEntity);
            old.selectedIndicator->SetShader(
                sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/base.fs"));
            old.selectedIndicator->SetHint(inactiveCol);

            // Stop forwarding cursor clicks to this actor
            old.cursorOnFloorClickSub.UnSubscribe();
            old.cursorOnEnemyLeftClickSub.UnSubscribe();
            old.cursorOnEnemyRightClickSub.UnSubscribe();
            old.cursorOnNPCLeftClickSub.UnSubscribe();
            old.cursorOnChestClickSub.UnSubscribe();
        }

        auto& current = registry->get<ControllableActor>(newEntity);
        current.selectedIndicator->SetHint(activeCol);
        current.selectedIndicator->SetShader(
            sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/base.fs"));

        // Forward cursor clicks to this actor's controllable component's events
        current.cursorOnFloorClickSub =
            sys->cursor->onFloorClick.Subscribe([this, newEntity](const entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(newEntity);
                c.onFloorClick.Publish(newEntity, clickedEntity);
            });
        current.cursorOnEnemyLeftClickSub =
            sys->cursor->onEnemyLeftClick.Subscribe([this, newEntity](const entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(newEntity);
                c.onEnemyLeftClick.Publish(newEntity, clickedEntity);
            });
        current.cursorOnEnemyRightClickSub =
            sys->cursor->onEnemyRightClick.Subscribe([this, newEntity](const entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(newEntity);
                c.onEnemyRightClick.Publish(newEntity, clickedEntity);
            });
        current.cursorOnNPCLeftClickSub =
            sys->cursor->onNPCClick.Subscribe([this, newEntity](const entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(newEntity);
                c.onNPCLeftClick.Publish(newEntity, clickedEntity);
            });
        current.cursorOnChestClickSub =
            sys->cursor->onChestClick.Subscribe([this, newEntity](entt::entity clickedEntity) {
                const auto& c = registry->get<ControllableActor>(newEntity);
                c.onChestClick.Publish(newEntity, clickedEntity);
            });

        for (const auto group = sys->partySystem->GetGroup(newEntity); const auto& entity : group)
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
        registry->emplace<PlayerState>(newEntity);
        for (const auto group = sys->partySystem->GetGroup(newEntity); const auto& entity : group)
        {
            if (entity != newEntity)
            {
                registry->emplace<PartyMemberState>(entity);
            }
        }
    }

    void ControllableActorSystem::onComponentAdded(entt::entity addedEntity)
    {
        auto& controllable = registry->get<ControllableActor>(addedEntity);
        controllable.selectedIndicator = std::make_unique<sage::TextureTerrainOverlay>(
            registry,
            sys->navigationGridSystem.get(),
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/particles/circle_03.png"),
            inactiveCol,
            "resources/shaders/glsl330/base.fs");
        auto& trans = registry->get<sage::sgTransform>(addedEntity);

        auto& collideable = registry->get<sage::Collideable>(addedEntity);
        auto r = (collideable.localBoundingBox.max.x - collideable.localBoundingBox.min.x) * 0.5f;
        r += (collideable.localBoundingBox.max.z - collideable.localBoundingBox.min.z) * 0.5f;

        // TODO: This is currently not centered correctly
        controllable.selectedIndicator->Init(trans.GetWorldPos(), r);
        controllable.selectedIndicator->Enable(true);
    }

    void ControllableActorSystem::onComponentRemoved(entt::entity removedEntity)
    {
    }

    ControllableActorSystem::ControllableActorSystem(entt::registry* _registry, Systems* _sys)
        : registry(_registry), sys(_sys)
    {
        registry->on_construct<ControllableActor>().connect<&ControllableActorSystem::onComponentAdded>(this);
        registry->on_destroy<ControllableActor>().connect<&ControllableActorSystem::onComponentRemoved>(this);
        sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity oldActor, entt::entity newActor) { setSelectedActor(oldActor, newActor); });
    }
} // namespace lq
