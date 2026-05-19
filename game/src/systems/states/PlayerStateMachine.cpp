#include "PlayerStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"
#include "collision/RpgCollisionLayers.hpp"

#include "Systems.hpp"

#include "AbilityFactory.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "components/DialogComponent.hpp"
#include "engine/Camera.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/systems/ActorMovementSystem.hpp"
#include "engine/systems/TransformSystem.hpp"
#include "raylib.h"
#include "systems/DialogSystem.hpp"
#include "systems/LootSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include <cassert>

namespace lq
{
    // ====== Click handlers ==========================================================

    void PlayerStateMachine::onFloorClick(const entt::entity entity, entt::entity)
    {
        ChangeState(entity, PlayerMovingToLocationState{});
    }

    void PlayerStateMachine::onChestClick(const entt::entity entity, const entt::entity target)
    {
        ChangeState(entity, PlayerMovingToLootState{.target = target});
    }

    void PlayerStateMachine::onNPCLeftClick(const entt::entity entity, const entt::entity target)
    {
        if (!registry->any_of<DialogComponent>(target)) return;
        ChangeState(entity, PlayerMovingToTalkState{.target = target});
    }

    void PlayerStateMachine::onEnemyLeftClick(const entt::entity entity, const entt::entity target)
    {
        registry->get<CombatableActor>(entity).target = target;
        ChangeState(entity, PlayerMovingToAttackEnemyState{});
    }

    // ====== Lifecycle ===============================================================

    void PlayerStateMachine::Update()
    {
    }

    void PlayerStateMachine::Draw3D()
    {
    }

    void PlayerStateMachine::bindCursorInput(const entt::entity entity)
    {
        auto& state = registry->get<PlayerState>(entity);
        auto& cursor = *sys->engine.cursor;

        state.BindPersistentSubscription(cursor.onNavigationClick.Subscribe(
            [this, entity](entt::entity target, sage::CollisionLayer) { onFloorClick(entity, target); }));
        state.BindPersistentSubscription(cursor.onLeftClick.Subscribe(
            [this, entity](entt::entity target, sage::CollisionLayer layer) {
                if (layer == collision_layers::Enemy)
                    onEnemyLeftClick(entity, target);
                else if (layer == collision_layers::Npc)
                    onNPCLeftClick(entity, target);
                else if (layer == collision_layers::Chest)
                    onChestClick(entity, target);
            }));
    }

    void PlayerStateMachine::onComponentAdded(const entt::entity entity)
    {
        bindCursorInput(entity);
        std::visit([this, entity](auto& cur) { cur.OnEnter(*this, entity); }, registry->get<PlayerState>(entity).current);
    }

    void PlayerStateMachine::onComponentRemoved(const entt::entity entity)
    {
        auto& state = registry->get<PlayerState>(entity);
        state.RemoveAllSubscriptions();
        state.RemovePersistentSubscriptions();
    }

    PlayerStateMachine::PlayerStateMachine(entt::registry* _registry, Systems* _sys) : Base(_registry), sys(_sys)
    {
        registry->on_construct<PlayerState>().connect<&PlayerStateMachine::onComponentAdded>(this);
        registry->on_destroy<PlayerState>().connect<&PlayerStateMachine::onComponentRemoved>(this);
    }
} // namespace lq
