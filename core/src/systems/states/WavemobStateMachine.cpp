#include "WavemobStateMachine.hpp"

#include "abilities/WavemobAutoAttack.hpp"
#include "components/CombatableActor.hpp"
#include "GameData.hpp"

#include "raylib.h"

namespace sage
{
    namespace enemystates
    {
        // ----------------------------
        void DefaultState::Update()
        {
        }

        void DefaultState::Draw3D()
        {
        }

        void DefaultState::OnHit(AttackData attackData)
        {
            auto& combatable = registry->get<CombatableActor>(attackData.hit);
            combatable.target = attackData.attacker;
            // What if it already has a target?
            ChangeState<StateEnemyCombat, EnemyStates>(attackData.hit);
        }

        void DefaultState::OnStateEnter(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            entt::sink sink{combatable.onHit};
            sink.connect<&DefaultState::OnHit>(this);

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

        void TargetOutOfRangeState::Update()
        {
            auto view = registry->view<CombatableActor, TargetOutOfRangeState>();
            for (const auto& entity : view)
            {
                auto& combatable = registry->get<CombatableActor>(entity);
                if (combatable.target == entt::null)
                {
                    ChangeState<StateEnemyDefault, EnemyStates>(entity);
                    continue;
                }
                if (isTargetOutOfSight(entity))
                {
                    ChangeState<StateEnemyDefault, EnemyStates>(entity);
                    continue;
                }
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
            auto collisions = gameData->collisionSystem->GetCollisionsWithRay(
                self, ray, collideable.collisionLayer);

            if (!collisions.empty() &&
                collisions.at(0).collisionLayer != CollisionLayer::PLAYER)
            {
                // Lost line of sight, out of combat
                combatable.target = entt::null;
                trans.movementDirectionDebugLine = {};
                return false;
            }
            return true;
        }

        void TargetOutOfRangeState::onTargetReached(entt::entity self)
        {
            ChangeState<StateEnemyCombat, EnemyStates>(self);
        }

        void TargetOutOfRangeState::OnStateEnter(entt::entity self)
        {
            auto& autoAttackAbility = registry->get<WavemobAutoAttack>(self);
            autoAttackAbility.Cancel();

            const auto& combatable = registry->get<CombatableActor>(self);
            const auto& target = registry->get<sgTransform>(combatable.target).position();

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

            gameData->actorMovementSystem->PathfindToLocation(self, target);

            auto& trans = registry->get<sgTransform>(self);
            entt::sink sink{trans.onFinishMovement};
            sink.connect<&TargetOutOfRangeState::onTargetReached>(this);
        }

        void TargetOutOfRangeState::OnStateExit(entt::entity self)
        {
            auto& trans = registry->get<sgTransform>(self);
            entt::sink sink{trans.onFinishMovement};
            sink.disconnect<&TargetOutOfRangeState::onTargetReached>(this);
        }

        TargetOutOfRangeState::TargetOutOfRangeState(
            entt::registry* _registry, GameData* _gameData)
            : StateMachine(_registry), gameData(_gameData)
        {
        }

        // ----------------------------

        void CombatState::Update()
        {
            auto view = registry->view<CombatableActor, StateEnemyCombat>();
            for (const auto& entity : view)
            {
                auto& combatable = registry->get<CombatableActor>(entity);
                if (!checkInCombat(entity))
                {
                    ChangeState<StateEnemyDefault, EnemyStates>(entity);
                    continue;
                }

                auto& actorTrans = registry->get<sgTransform>(entity);
                auto target = registry->get<sgTransform>(combatable.target).position();
                float distance = Vector3Distance(actorTrans.position(), target);

                // TODO: Arbitrary number. Should probably use the navigation system to
                // find the "next best square" from current position
                if (distance >= 8.0f)
                {
                    ChangeState<StateEnemyTargetOutOfRange, EnemyStates>(entity);
                    continue;
                }

                auto& autoAttackAbility = registry->get<WavemobAutoAttack>(entity);
                autoAttackAbility.Update(entity);
            }
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
            autoAttackAbility.Cancel();
        }

        CombatState::CombatState(entt::registry* _registry) : StateMachine(_registry)
        {
        }

        // ----------------------------

        void DyingState::Update()
        {
            auto view = registry->view<CombatableActor>();
            for (const auto& entity : view)
            {
                auto& combatable = registry->get<CombatableActor>(entity);
                if (combatable.actorType != CombatableActorType::WAVEMOB)
                {
                    continue;
                }
                if (combatable.hp <= 0)
                {
                    if (!registry->any_of<StateEnemyDying>(entity))
                    {
                        ChangeState<StateEnemyDying, EnemyStates>(entity);
                    }
                }
            }
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

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
            {
                entt::sink sink{animation.onAnimationEnd};
                sink.connect<&DyingState::destroyEntity>(this);
            }
            auto& autoAttackAbility = registry->get<WavemobAutoAttack>(self);
            autoAttackAbility.Cancel();
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

    void WavemobStateController::Update()
    {
        for (auto& system : systems)
        {
            system->Update();
        }
    }

    void WavemobStateController::Draw3D()
    {
        for (auto& system : systems)
        {
            system->Draw3D();
        }
    }

    WavemobStateController::WavemobStateController(
        entt::registry* _registry, GameData* _gameData)
        : registry(_registry)
    {
        defaultState = std::make_unique<enemystates::DefaultState>(_registry, _gameData);
        targetOutOfRangeState =
            std::make_unique<enemystates::TargetOutOfRangeState>(_registry, _gameData);
        engagedInCombatState = std::make_unique<enemystates::CombatState>(_registry);
        dyingState = std::make_unique<enemystates::DyingState>(_registry, _gameData);

        systems.push_back(defaultState.get());
        systems.push_back(targetOutOfRangeState.get());
        systems.push_back(engagedInCombatState.get());
        systems.push_back(dyingState.get());
    }
} // namespace sage