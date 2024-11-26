#include "WavemobStateMachine.hpp"

#include "GameData.hpp"

#include "AbilityFactory.hpp"
#include "components/Ability.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "EntityReflectionSignalRouter.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include "raylib.h"

namespace sage
{
    class WavemobStateController::DefaultState : public StateMachine
    {
        WavemobStateController* stateController;

      public:
        void OnDeath(entt::entity entity)
        {
            stateController->ChangeState(entity, WavemobStateEnum::Dying);
        }

        void OnHit(AttackData attackData)
        {
            auto& combatable = registry->get<CombatableActor>(attackData.hit);
            combatable.target = attackData.attacker;
            // What if it already has a target?
            auto& state = registry->get<WavemobState>(attackData.hit);
            if (state.GetCurrentState() != WavemobStateEnum::Dying)
            {
                stateController->ChangeState(attackData.hit, WavemobStateEnum::Combat);
            }
        }

        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity self) override
        {
            auto& state = registry->get<WavemobState>(self);
            auto& combatable = registry->get<CombatableActor>(self);
            entt::sink sink{combatable.onHit};
            state.AddConnection(sink.connect<&DefaultState::OnHit>(this));
            // Persistent connections
            entt::sink deathSink{combatable.onDeath};
            deathSink.connect<&DefaultState::OnDeath>(this);
            // ----------------------------
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnStateExit(entt::entity entity) override
        {
        }

        ~DefaultState() override = default;

        DefaultState(entt::registry* registry, GameData* _gameData, WavemobStateController* _stateController)
            : StateMachine(registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class WavemobStateController::TargetOutOfRangeState : public StateMachine
    {
        WavemobStateController* stateController;

        void onTargetReached(entt::entity self)
        {
            stateController->ChangeState(self, WavemobStateEnum::Combat);
        }

        bool isTargetOutOfSight(entt::entity self)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            auto& trans = registry->get<sgTransform>(self);
            auto& collideable = registry->get<Collideable>(self);

            const auto& targetPos = registry->get<sgTransform>(combatable.target).GetWorldPos();
            Vector3 direction = Vector3Subtract(targetPos, trans.GetWorldPos());
            float distance = Vector3Distance(trans.GetWorldPos(), targetPos);
            Vector3 normDirection = Vector3Normalize(direction);

            Ray ray;
            ray.position = trans.GetWorldPos();
            ray.direction = Vector3Scale(normDirection, distance);
            float height = Vector3Subtract(collideable.localBoundingBox.max, collideable.localBoundingBox.min).y;
            ray.position.y = trans.GetWorldPos().y + height;
            ray.direction.y = trans.GetWorldPos().y + height;
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

        void onTargetPosUpdate(entt::entity self, entt::entity target) const
        {
            const auto& targetPos = registry->get<sgTransform>(target).GetWorldPos();
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::WALK, 2);
            gameData->actorMovementSystem->PathfindToLocation(self, targetPos);
        }

      public:
        void Update(entt::entity entity) override
        {
            const auto& combatable = registry->get<CombatableActor>(entity);
            if (combatable.target == entt::null || isTargetOutOfSight(entity))
            {
                stateController->ChangeState(entity, WavemobStateEnum::Default);
            }
        }

        void OnStateEnter(entt::entity self) override
        {
            const auto abilityEntity = gameData->abilityRegistry->GetAbility(self, AbilityEnum::ENEMY_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.publish(abilityEntity);

            auto& moveable = registry->get<MoveableActor>(self);
            const auto& combatable = registry->get<CombatableActor>(self);
            moveable.followTarget.emplace(registry, self, combatable.target);

            auto& state = registry->get<WavemobState>(self);

            entt::sink finishMovementSink{moveable.onDestinationReached};
            state.AddConnection(finishMovementSink.connect<&TargetOutOfRangeState::onTargetReached>(this));
            entt::sink posUpdateSink{moveable.followTarget->onPathChanged};
            state.AddConnection(posUpdateSink.connect<&TargetOutOfRangeState::onTargetPosUpdate>(this));

            onTargetPosUpdate(self, combatable.target);
        }

        void OnStateExit(entt::entity self) override
        {
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
        }

        ~TargetOutOfRangeState() override = default;

        TargetOutOfRangeState(
            entt::registry* registry, GameData* _gameData, WavemobStateController* _stateController)
            : StateMachine(registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class WavemobStateController::CombatState : public StateMachine
    {
        WavemobStateController* stateController;

        void onTargetDeath(entt::entity self, entt::entity target)
        {
        }

        bool checkInCombat(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            return !combatable.dying && combatable.target != entt::null;
        }

      public:
        void Update(entt::entity self) override
        {
            auto& combatable = registry->get<CombatableActor>(self);
            auto& state = registry->get<WavemobState>(self);
            if (!checkInCombat(self))
            {
                stateController->ChangeState(self, WavemobStateEnum::Default);
                return;
            }
            auto& actorTrans = registry->get<sgTransform>(self);
            auto target = registry->get<sgTransform>(combatable.target).GetWorldPos();
            float distance = Vector3Distance(actorTrans.GetWorldPos(), target);
            // TODO: Arbitrary number. Should probably use the navigation system to
            // find the "next best square" from current position
            if (distance >= 8.0f)
            {
                stateController->ChangeState(self, WavemobStateEnum::TargetOutOfRange);
                return;
            }
        }

        void OnStateEnter(entt::entity entity) override
        {
            auto abilityEntity = gameData->abilityRegistry->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
            registry->get<Ability>(abilityEntity).startCast.publish(abilityEntity);
        }

        void OnStateExit(entt::entity entity) override
        {
            auto abilityEntity = gameData->abilityRegistry->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.publish(abilityEntity);
        }

        ~CombatState() override = default;

        CombatState(entt::registry* registry, GameData* _gameData, WavemobStateController* _stateController)
            : StateMachine(registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class WavemobStateController::DyingState : public StateMachine
    {
        WavemobStateController* stateController;

        void destroyEntity(entt::entity self)
        {
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
            {
                entt::sink sink{animation.onAnimationEnd};
                sink.disconnect<&DyingState::destroyEntity>(this);
            }
            registry->destroy(self);
        }

      public:
        void OnStateEnter(entt::entity self) override
        {
            LockState(self); // Target is dying, do not change state
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            combatable.dying = true;
            auto& bb = registry->get<Collideable>(self).worldBoundingBox;
            gameData->navigationGridSystem->MarkSquareAreaOccupied(bb, false);
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
            auto& state = registry->get<WavemobState>(self);
            entt::sink sink{animation.onAnimationEnd};
            state.AddConnection(sink.connect<&DyingState::destroyEntity>(this));

            auto abilityEntity = gameData->abilityRegistry->GetAbility(self, AbilityEnum::ENEMY_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.publish(abilityEntity);

            gameData->actorMovementSystem->CancelMovement(self);
        }

        void OnStateExit(entt::entity self) override
        {
            UnlockState(self);
        }

        ~DyingState() override = default;

        DyingState(entt::registry* registry, GameData* gameData, WavemobStateController* _stateController)
            : StateMachine(registry, gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void WavemobStateController::Update()
    {
        for (const auto view = registry->view<WavemobState>(); const auto& entity : view)
        {
            const auto state = registry->get<WavemobState>(entity).GetCurrentState();
            GetSystem(state)->Update(entity);
        }
    }

    void WavemobStateController::Draw3D()
    {
        for (const auto view = registry->view<WavemobState>(); const auto& entity : view)
        {
            const auto state = registry->get<WavemobState>(entity).GetCurrentState();
            GetSystem(state)->Draw3D(entity);
        }
    }

    WavemobStateController::WavemobStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry)
    {
        states[WavemobStateEnum::Default] = std::make_unique<DefaultState>(_registry, _gameData, this);
        states[WavemobStateEnum::TargetOutOfRange] =
            std::make_unique<TargetOutOfRangeState>(_registry, _gameData, this);
        states[WavemobStateEnum::Combat] = std::make_unique<CombatState>(_registry, _gameData, this);
        states[WavemobStateEnum::Dying] = std::make_unique<DyingState>(_registry, _gameData, this);
    }
} // namespace sage