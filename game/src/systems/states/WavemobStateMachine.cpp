#include "WavemobStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "Systems.hpp"

#include "AbilityFactory.hpp"
#include "collision/RpgCollisionLayers.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"

#include "engine/components/Animation.hpp"
#include "engine/components/DeleteEntityComponent.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/ActorMovementSystem.hpp"
#include "engine/systems/CollisionSystem.hpp"
#include "engine/systems/NavigationGridSystem.hpp"

#include "raylib.h"

namespace lq
{
    bool WavemobStateMachine::isTargetOutOfSight(const entt::entity entity) const
    {
        auto& combatable = registry->get<CombatableActor>(entity);
        auto& trans = registry->get<sage::sgTransform>(entity);
        auto& collideable = registry->get<sage::Collideable>(entity);

        const auto& targetPos = registry->get<sage::sgTransform>(combatable.target).GetWorldPos();
        Vector3 direction = Vector3Subtract(targetPos, trans.GetWorldPos());
        const float distance = Vector3Distance(trans.GetWorldPos(), targetPos);
        const Vector3 normDirection = Vector3Normalize(direction);

        Ray ray;
        ray.position = trans.GetWorldPos();
        ray.direction = Vector3Scale(normDirection, distance);
        const float height = Vector3Subtract(collideable.localBoundingBox.max, collideable.localBoundingBox.min).y;
        ray.position.y = trans.GetWorldPos().y + height;
        ray.direction.y = trans.GetWorldPos().y + height;
        trans.movementDirectionDebugLine = ray;

        const auto collisions =
            sys->engine.collisionSystem->GetCollisionsWithRay(entity, ray, collideable.collidesWith);

        if (!collisions.empty() && collisions.at(0).collisionLayer != lq::collision_layers::Player)
        {
            // Lost line of sight, out of combat
            combatable.target = entt::null;
            trans.movementDirectionDebugLine = {};
            return true;
        }
        return false;
    }

    void WavemobStateMachine::onTargetPosUpdate(const entt::entity entity, const entt::entity target) const
    {
        const auto& targetPos = registry->get<sage::sgTransform>(target).GetWorldPos();
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Walk, 2);
        sys->engine.actorMovementSystem->PathfindToLocation(entity, targetPos);
    }

    void WavemobStateMachine::destroyEntity(const entt::entity entity)
    {
        registry->get<WavemobState>(entity).RemoveAllSubscriptions();
        registry->emplace<sage::DeleteEntityComponent>(entity);
    }

    // ====== Cross-state handlers ====================================================

    void WavemobStateMachine::onHit(const AttackData attackData)
    {
        auto& combatable = registry->get<CombatableActor>(attackData.hit);
        combatable.target = attackData.attacker;
        ChangeState(attackData.hit, WavemobCombatState{});
    }

    void WavemobStateMachine::onDeath(const entt::entity entity)
    {
        ChangeState(entity, WavemobDyingState{});
    }

    // ====== Lifecycle ===============================================================

    void WavemobStateMachine::Update()
    {
        for (const auto view = registry->view<WavemobState>(); const auto& entity : view)
        {
            auto& state = registry->get<WavemobState>(entity);
            std::visit([this, entity](auto& cur) { cur.Update(*this, entity); }, state.current);
        }
    }

    void WavemobStateMachine::Draw3D()
    {
    }

    void WavemobStateMachine::onComponentAdded(const entt::entity entity)
    {
        auto& combatable = registry->get<CombatableActor>(entity);
        // Persistent subscriptions — survive state transitions, freed implicitly when the
        // combatable component (or the entity) is destroyed.
        combatable.onHit.Subscribe([this](const AttackData ad) { onHit(ad); });
        combatable.onDeath.Subscribe([this](const entt::entity e) { onDeath(e); });

        auto& state = registry->get<WavemobState>(entity);
        std::visit([this, entity](auto& cur) { cur.OnEnter(*this, entity); }, state.current);
    }

    WavemobStateMachine::WavemobStateMachine(entt::registry* _registry, Systems* _sys)
        : Base(_registry), sys(_sys)
    {
        registry->on_construct<WavemobState>().connect<&WavemobStateMachine::onComponentAdded>(this);
        registry->on_destroy<WavemobState>().connect<&WavemobStateMachine::onComponentRemoved>(this);
    }
} // namespace lq
