#include "AbilityStateMachine.hpp"

#include "abilities/AbilityData.hpp"
#include "abilities/AbilityFunctions.hpp"
#include "abilities/AbilityIndicator.hpp"
#include "abilities/vfx/VisualFX.hpp"
#include "AbilityFactory.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "engine/Timer.hpp"
#include "GameObjectFactory.hpp"
#include "Systems.hpp"

#include "../ControllableActorSystem.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/systems/TransformSystem.hpp"
#include "engine/TextureTerrainOverlay.hpp"

#include "raylib.h"

#include <cassert>
#include <iostream>

namespace lq
{
    void AbilityStateMachine::enableCursor(const entt::entity entity)
    {
        auto& ab = registry->get<Ability>(entity);
        ab.abilityIndicator->Init(sys->engine.cursor->getFirstNaviCollision().point);
        ab.abilityIndicator->Enable(true);
        sys->engine.cursor->Disable();
        sys->engine.cursor->Hide();
    }

    void AbilityStateMachine::disableCursor(const entt::entity entity)
    {
        auto& ab = registry->get<Ability>(entity);
        sys->engine.cursor->Enable();
        sys->engine.cursor->Show();
        ab.abilityIndicator->Enable(false);
    }

    // ====== Cross-state transitions =================================================

    void AbilityStateMachine::cancelCast(const entt::entity entity)
    {
        auto& ab = registry->get<Ability>(entity);
        if (auto* vfx = ab.GetVfx(registry); vfx && vfx->active)
        {
            vfx->active = false;
        }
        ab.cooldownTimer.Stop();
        ab.castTimer.Stop();
        ChangeState(entity, AbilityIdleState{});
    }

    void AbilityStateMachine::executeAbility(const entt::entity entity)
    {
        const auto& ab = registry->get<Ability>(entity);
        const auto& ad = registry->get<AbilityData>(entity);

        if (ad.base.HasBehaviour(AbilityBehaviour::ATTACK_TARGET))
        {
            const auto target = registry->get<CombatableActor>(ab.caster).target;
            HitSingleTarget(registry, sys->Engine(), ab.caster, entity, target);
        }
        else if (ad.base.HasBehaviour(AbilityBehaviour::ATTACK_AOE_POINT))
        {
            Vector3 targetPos{};
            if (ad.base.HasBehaviour(AbilityBehaviour::FOLLOW_NONE))
            {
                targetPos = registry->get<sage::sgTransform>(entity).GetWorldPos();
            }
            else if (ad.base.HasBehaviour(AbilityBehaviour::FOLLOW_CASTER))
            {
                targetPos = registry->get<sage::sgTransform>(ab.caster).GetWorldPos();
            }
            AOEAtPoint(registry, ab.caster, entity, targetPos, ad.base.radius);
        }

        ChangeState(entity, AbilityIdleState{});
    }

    bool AbilityStateMachine::checkRange(const entt::entity entity) const
    {
        // TODO: Should account for more possibilities with flags here.
        const auto& ab = registry->get<Ability>(entity);
        const auto& ad = registry->get<AbilityData>(entity);
        if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CURSOR))
        {
            const auto& casterPos = registry->get<sage::sgTransform>(ab.caster).GetWorldPos();
            if (const auto point = sys->engine.cursor->getFirstNaviCollision().point;
                Vector3Distance(point, casterPos) > ad.base.range)
            {
                std::cout << "Out of range. \n";
                ab.castFailed.Publish(entity, AbilityCastFail::OUT_OF_RANGE);
                return false;
            }
        }
        return true;
    }

    void AbilityStateMachine::spawnAbility(const entt::entity entity)
    {
        const auto& ab = registry->get<Ability>(entity);
        const auto& ad = registry->get<AbilityData>(entity);

        if (!checkRange(entity)) return;

        auto& animation = registry->get<sage::Animation>(ab.caster);
        animation.ChangeAnimationByParams(ad.animationParams);
        if (auto* vfx = ab.GetVfx(registry))
        {
            const auto& trans = registry->get<sage::sgTransform>(entity);
            if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CASTER))
            {
                const auto& casterTrans = registry->get<sage::sgTransform>(ab.caster);
                const auto& [min, max] = registry->get<sage::Collideable>(ab.caster).worldBoundingBox;
                const float heightOffset = Vector3Subtract(max, min).y;

                if (ad.base.HasBehaviour(AbilityBehaviour::FOLLOW_CASTER))
                {
                    // TODO: Can this be set in the ability's constructor?
                    // Then we can just say "if ability doesn't follow caster, then set its position"
                    if (trans.GetParent() != ab.caster)
                    {
                        sys->engine.registry->get<sage::sgTransform>(entity)
                            .SetParent(ab.caster, &sys->engine.registry->get<sage::sgTransform>(ab.caster));
                        sys->engine.registry->get<sage::sgTransform>(entity).SetLocalPos({0, heightOffset, 0});
                        sys->engine.registry->get<sage::sgTransform>(entity).SetLocalRot(Vector3Zero());
                    }
                }
                else
                {
                    const Vector3 pos{casterTrans.GetWorldPos().x, heightOffset, casterTrans.GetWorldPos().z};
                    sys->engine.registry->get<sage::sgTransform>(entity).SetWorldPos(pos);
                    sys->engine.registry->get<sage::sgTransform>(entity).SetWorldRot(casterTrans.GetWorldRot());
                }

                vfx->InitSystem();
            }
            else if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CURSOR))
            {
                sys->engine.registry->get<sage::sgTransform>(entity).SetWorldPos(sys->engine.cursor->getFirstNaviCollision().point);
                vfx->InitSystem();
            }
        }

        ChangeState(entity, AbilityAwaitingExecutionState{});
    }

    // Determines if we need to display an indicator or not
    void AbilityStateMachine::startCast(const entt::entity entity)
    {
        const auto& ad = registry->get<AbilityData>(entity);
        if (ad.base.HasOptionalBehaviour(AbilityBehaviourOptional::INDICATOR))
        {
            auto& state = registry->get<AbilityState>(entity);
            if (std::holds_alternative<AbilityCursorSelectState>(state.current))
            {
                ChangeState(entity, AbilityIdleState{});
            }
            else
            {
                ChangeState(entity, AbilityCursorSelectState{});
            }
            return;
        }
        spawnAbility(entity);
    }

    // ====== Lifecycle ===============================================================

    void AbilityStateMachine::Update()
    {
        for (const auto view = registry->view<AbilityState, Ability>(); const auto entity : view)
        {
            auto& state = registry->get<AbilityState>(entity);
            auto& ab = registry->get<Ability>(entity);

            // Skip idle, inactive abilities; cursor-select always needs ticking for input.
            const bool inCursorSelect = std::holds_alternative<AbilityCursorSelectState>(state.current);
            if (!ab.IsActive() && !inCursorSelect) continue;

            std::visit([this, entity](auto& cur) { cur.Update(*this, entity); }, state.current);

            if (auto* vfx = ab.GetVfx(registry); vfx && vfx->active)
            {
                vfx->Update(GetFrameTime());
            }
        }
    }

    void AbilityStateMachine::Draw3D()
    {
        for (const auto view = registry->view<AbilityState, Ability>(); const auto entity : view)
        {
            const auto& state = registry->get<AbilityState>(entity);
            auto& ab = registry->get<Ability>(entity);

            const bool inCursorSelect = std::holds_alternative<AbilityCursorSelectState>(state.current);
            if (!ab.IsActive() && !inCursorSelect) continue;

            if (auto* vfx = ab.GetVfx(registry); vfx && vfx->active)
            {
                vfx->Draw3D();
            }
        }
    }

    void AbilityStateMachine::onComponentAdded(const entt::entity entity)
    {
        auto& ab = registry->get<Ability>(entity);
        // Persistent subscriptions — survive state transitions, freed implicitly when the
        // Ability component (or the entity) is destroyed.
        ab.startCast.Subscribe([this](const entt::entity e) { startCast(e); });
        ab.cancelCast.Subscribe([this](const entt::entity e) { cancelCast(e); });

        auto& state = registry->get<AbilityState>(entity);
        std::visit([this, entity](auto& cur) { cur.OnEnter(*this, entity); }, state.current);
    }

    AbilityStateMachine::AbilityStateMachine(entt::registry* _registry, Systems* _sys)
        : Base(_registry), sys(_sys)
    {
        registry->on_construct<Ability>().connect<&AbilityStateMachine::onComponentAdded>(this);
        registry->on_destroy<Ability>().connect<&AbilityStateMachine::onComponentRemoved>(this);
    }
} // namespace lq
