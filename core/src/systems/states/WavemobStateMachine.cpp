#include "WavemobStateMachine.hpp"

#include "GameData.hpp"

#include "abilities/Abilities.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/MovableActor.hpp"
#include "components/sgTransform.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include "raylib.h"

namespace sage
{
    namespace enemystates
    {
        // ----------------------------
        void DefaultState::Update(entt::entity entity)
        {
        }

        void DefaultState::Draw3D(entt::entity entity)
        {
        }

        void DefaultState::OnDeath(entt::entity entity)
        {
            auto& state = registry->get<WavemobState>(entity);
            state.ChangeState(entity, WavemobStateEnum::Dying);
        }

        void DefaultState::OnHit(AttackData attackData)
        {
            auto& combatable = registry->get<CombatableActor>(attackData.hit);
            combatable.target = attackData.attacker;
            // What if it already has a target>
            auto& state = registry->get<WavemobState>(attackData.hit);
            if (state.GetCurrentState() != WavemobStateEnum::Dying)
            {
                state.ChangeState(attackData.hit, WavemobStateEnum::Combat);
            }
        }

        void DefaultState::OnStateEnter(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            entt::sink sink{combatable.onHit};
            sink.connect<&DefaultState::OnHit>(this);

            // Persistent connection
            entt::sink deathSink{combatable.onDeath};
            deathSink.connect<&DefaultState::OnDeath>(this);
            // ----------------------------

            Vector3 target = {52, 0, -10}; // TODO: Just a random location for now
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
            gameData->actorMovementSystem->PathfindToLocation(entity, target);
        }

        void DefaultState::OnStateExit(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            entt::sink sink{combatable.onHit};
            sink.disconnect<&DefaultState::OnHit>(this);
            gameData->actorMovementSystem->CancelMovement(entity);
        }

        DefaultState::DefaultState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------

        void TargetOutOfRangeState::Update(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            if (combatable.target == entt::null || isTargetOutOfSight(entity))
            {
                auto& state = registry->get<WavemobState>(entity);
                state.ChangeState(entity, WavemobStateEnum::Default);
            }
        }

        bool TargetOutOfRangeState::isTargetOutOfSight(entt::entity self)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            auto& trans = registry->get<sgTransform>(self);
            const auto& target = registry->get<sgTransform>(combatable.target).position();
            Vector3 direction = Vector3Subtract(target, trans.position());
            float distance = Vector3Distance(trans.position(), target);
            Vector3 normDirection = Vector3Normalize(direction);

            auto& collideable = registry->get<Collideable>(self);

            Ray ray;
            ray.position = trans.position();
            ray.direction = Vector3Scale(normDirection, distance);
            ray.position.y = 0.5f;
            ray.direction.y = 0.5f;
            trans.movementDirectionDebugLine = ray;
            auto collisions =
                gameData->collisionSystem->GetCollisionsWithRay(self, ray, collideable.collisionLayer);

            if (!collisions.empty() && collisions.at(0).collisionLayer != CollisionLayer::PLAYER)
            {
                // Lost line of sight, out of combat
                combatable.target = entt::null;
                trans.movementDirectionDebugLine = {};
                return true;
            }
            return false;
        }

        void TargetOutOfRangeState::onTargetReached(entt::entity self)
        {
            auto& state = registry->get<WavemobState>(self);
            state.ChangeState(self, WavemobStateEnum::Combat);
        }

        void TargetOutOfRangeState::OnStateEnter(entt::entity self)
        {
            auto& autoAttackAbility = registry->get<WavemobAutoAttack>(self);
            autoAttackAbility.Cancel(self);

            const auto& combatable = registry->get<CombatableActor>(self);
            const auto& target = registry->get<sgTransform>(combatable.target).position();

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

            gameData->actorMovementSystem->PathfindToLocation(self, target);

            auto& moveableActor = registry->get<MoveableActor>(self);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.connect<&TargetOutOfRangeState::onTargetReached>(this);
        }

        void TargetOutOfRangeState::OnStateExit(entt::entity self)
        {
            auto& moveableActor = registry->get<MoveableActor>(self);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.disconnect<&TargetOutOfRangeState::onTargetReached>(this);
        }

        TargetOutOfRangeState::TargetOutOfRangeState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------

        void CombatState::Update(entt::entity self)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            auto& state = registry->get<WavemobState>(self);
            if (!checkInCombat(self))
            {
                state.ChangeState(self, WavemobStateEnum::Default);
                return;
            }

            auto& actorTrans = registry->get<sgTransform>(self);
            auto target = registry->get<sgTransform>(combatable.target).position();
            float distance = Vector3Distance(actorTrans.position(), target);

            // TODO: Arbitrary number. Should probably use the navigation system to
            // find the "next best square" from current position
            if (distance >= 8.0f)
            {
                state.ChangeState(self, WavemobStateEnum::TargetOutOfRange);
                return;
            }

            auto& autoAttackAbility = registry->get<WavemobAutoAttack>(self);
            autoAttackAbility.Update(self);
        }

        bool CombatState::checkInCombat(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            return !combatable.dying && combatable.target != entt::null;
        }

        void CombatState::OnStateEnter(entt::entity entity)
        {
            auto& autoAttackAbility = registry->get<WavemobAutoAttack>(entity);
            autoAttackAbility.Init(entity);
        }

        void CombatState::OnStateExit(entt::entity entity)
        {
            auto& autoAttackAbility = registry->get<WavemobAutoAttack>(entity);
            autoAttackAbility.Cancel(entity);
        }

        CombatState::CombatState(entt::registry* _registry) : StateMachine(_registry)
        {
        }

        // ----------------------------

        void DyingState::Update(entt::entity self) // TODO: this will not work currently
        {
        }

        void DyingState::destroyEntity(entt::entity self)
        {
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
            {
                entt::sink sink{animation.onAnimationEnd};
                sink.disconnect<&DyingState::destroyEntity>(this);
            }
            registry->destroy(self);
        }

        void DyingState::OnStateEnter(entt::entity self)
        {
            LockState(self); // Target is dying, do not change state
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            combatable.dying = true;

            auto& bb = registry->get<Collideable>(self).worldBoundingBox;
            gameData->navigationGridSystem->MarkSquareAreaOccupied(bb, false);

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
            {
                entt::sink sink{animation.onAnimationEnd};
                sink.connect<&DyingState::destroyEntity>(this);
            }
            auto& autoAttackAbility = registry->get<WavemobAutoAttack>(self);
            autoAttackAbility.Cancel(self);
            gameData->actorMovementSystem->CancelMovement(self);
        }

        void DyingState::OnStateExit(entt::entity self)
        {
            UnlockState(self);
        }

        DyingState::DyingState(entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------

    } // namespace enemystates

    StateMachine* WavemobStateController::GetSystem(WavemobStateEnum state)
    {
        switch (state)
        {
        case WavemobStateEnum::Default:
            return defaultState.get();
        case WavemobStateEnum::TargetOutOfRange:
            return targetOutOfRangeState.get();
        case WavemobStateEnum::Combat:
            return combatState.get();
        case WavemobStateEnum::Dying:
            return dyingState.get();
        default:
            return defaultState.get();
        }
    }

    void WavemobStateController::Update()
    {
        auto view = registry->view<WavemobState>();
        for (const auto& entity : view)
        {
            auto state = registry->get<WavemobState>(entity).GetCurrentState();
            GetSystem(state)->Update(entity);
        }
    }

    void WavemobStateController::Draw3D()
    {
        auto view = registry->view<WavemobState>();
        for (const auto& entity : view)
        {
            auto state = registry->get<WavemobState>(entity).GetCurrentState();
            GetSystem(state)->Draw3D(entity);
        }
    }

    WavemobStateController::WavemobStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry)
    {
        defaultState = std::make_unique<enemystates::DefaultState>(_registry, _gameData);
        targetOutOfRangeState = std::make_unique<enemystates::TargetOutOfRangeState>(_registry, _gameData);
        combatState = std::make_unique<enemystates::CombatState>(_registry);
        dyingState = std::make_unique<enemystates::DyingState>(_registry, _gameData);
    }
} // namespace sage