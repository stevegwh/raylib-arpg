#include "WavemobStateMachine.hpp"

#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include "Cursor.hpp"

#include "abilities/WavemobAutoAttack.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/HealthBar.hpp"
#include "components/sgTransform.hpp"

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
            auto& c = registry->get<CombatableActor>(attackData.hit);
            c.target = attackData.attacker;
            // What if it already has a target?
            ChangeState<StateEnemyCombat, EnemyStates>(attackData.hit);
        }

        void DefaultState::OnStateEnter(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            entt::sink sink{combatable.onHit};
            sink.connect<&DefaultState::OnHit>(this);

            Vector3 target = {52, 0, -10}; // TODO: Just a random location for now
            auto& a = registry->get<MoveableActor>(entity);
            auto& t = registry->get<sgTransform>(entity);
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
            actorMovementSystem->PathfindToLocation(entity, target);
        }

        void DefaultState::OnStateExit(entt::entity entity)
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            entt::sink sink{combatable.onHit};
            sink.disconnect<&DefaultState::OnHit>(this);
            actorMovementSystem->CancelMovement(entity);
        }

        DefaultState::DefaultState(
            entt::registry* _registry, ActorMovementSystem* _actorMovementSystem)
            : StateMachine(_registry), actorMovementSystem(_actorMovementSystem)
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
            auto& actorTrans = registry->get<sgTransform>(self);
            auto& target = registry->get<sgTransform>(combatable.target).position();
            Vector3 direction = Vector3Subtract(target, actorTrans.position());
            float distance = Vector3Distance(actorTrans.position(), target);
            Vector3 normDirection = Vector3Normalize(direction);

            auto& collideable = registry->get<Collideable>(self);

            Ray ray;
            ray.position = actorTrans.position();
            ray.direction = Vector3Scale(normDirection, distance);
            ray.position.y = 0.5f;
            ray.direction.y = 0.5f;
            actorTrans.movementDirectionDebugLine = ray;
            auto collisions = collisionSystem->GetCollisionsWithRay(
                self, ray, collideable.collisionLayer);

            if (!collisions.empty() &&
                collisions.at(0).collisionLayer != CollisionLayer::PLAYER)
            {
                // Lost line of sight, out of combat
                combatable.target = entt::null;
                actorTrans.movementDirectionDebugLine = {};
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
            auto& animation = registry->get<Animation>(self);
            auto& combatableActor = registry->get<CombatableActor>(self);
            auto& target = registry->get<sgTransform>(combatableActor.target).position();

            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

            actorMovementSystem->PathfindToLocation(self, target);

            auto& playerTrans = registry->get<sgTransform>(self);
            entt::sink sink{playerTrans.onFinishMovement};
            sink.connect<&TargetOutOfRangeState::onTargetReached>(this);
        }

        void TargetOutOfRangeState::OnStateExit(entt::entity self)
        {
            auto& playerTrans = registry->get<sgTransform>(self);
            entt::sink sink{playerTrans.onFinishMovement};
            sink.disconnect<&TargetOutOfRangeState::onTargetReached>(this);
        }

        TargetOutOfRangeState::TargetOutOfRangeState(
            entt::registry* _registry,
            ControllableActorSystem* _controllableActorSystem,
            ActorMovementSystem* _actorMovementSystem,
            CollisionSystem* _collisionSystem)
            : StateMachine(_registry),
              controllableActorSystem(_controllableActorSystem),
              actorMovementSystem(_actorMovementSystem),
              collisionSystem(_collisionSystem)
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
            actorMovementSystem->CancelMovement(self);
        }

        void DyingState::OnStateExit(entt::entity self)
        {
        }

        DyingState::DyingState(
            entt::registry* _registry, ActorMovementSystem* _actorMovementSystem)
            : StateMachine(_registry), actorMovementSystem(_actorMovementSystem)
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
        entt::registry* _registry,
        Cursor* _cursor,
        ControllableActorSystem* _controllableActorSystem,
        ActorMovementSystem* _actorMovementSystem,
        CollisionSystem* _collisionSystem,
        NavigationGridSystem* _navigationGridSystem)
        : registry(_registry),
          cursor(_cursor),
          controllableActorSystem(_controllableActorSystem)
    {
        defaultState =
            std::make_unique<enemystates::DefaultState>(_registry, _actorMovementSystem);
        targetOutOfRangeState = std::make_unique<enemystates::TargetOutOfRangeState>(
            _registry, _controllableActorSystem, _actorMovementSystem, _collisionSystem);
        engagedInCombatState = std::make_unique<enemystates::CombatState>(_registry);
        dyingState =
            std::make_unique<enemystates::DyingState>(_registry, _actorMovementSystem);

        systems.push_back(defaultState.get());
        systems.push_back(targetOutOfRangeState.get());
        systems.push_back(engagedInCombatState.get());
        systems.push_back(dyingState.get());
    }
} // namespace sage