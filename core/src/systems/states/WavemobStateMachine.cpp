#include "WavemobStateMachine.hpp"

#include "Systems.hpp"

#include "AbilityFactory.hpp"
#include "components/Ability.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include "components/DeleteEntityComponent.hpp"
#include "raylib.h"

namespace sage
{
    class WavemobStateMachine::DefaultState : public State
    {
        WavemobStateMachine* stateController;

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
            if (state.GetCurrentStateEnum() != WavemobStateEnum::Dying)
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

        void OnEnter(entt::entity self) override
        {
            auto& state = registry->get<WavemobState>(self);
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.onHit.Subscribe([this](const AttackData ad) { OnHit(ad); });
            // Persistent subscriptions
            combatable.onDeath.Subscribe([this](const entt::entity entity) { OnDeath(entity); });
            // ----------------------------
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnExit(entt::entity entity) override
        {
        }

        ~DefaultState() override = default;

        DefaultState(entt::registry* registry, Systems* _sys, WavemobStateMachine* _stateController)
            : State(registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class WavemobStateMachine::TargetOutOfRangeState : public State
    {
        WavemobStateMachine* stateController;

        void onTargetReached(entt::entity self) const
        {
            stateController->ChangeState(self, WavemobStateEnum::Combat);
        }

        [[nodiscard]] bool isTargetOutOfSight(entt::entity self) const
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

            auto collisions = sys->collisionSystem->GetCollisionsWithRay(self, ray, collideable.collisionLayer);

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
            sys->actorMovementSystem->PathfindToLocation(self, targetPos);
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

        void OnEnter(entt::entity self) override
        {
            const auto abilityEntity = sys->abilityFactory->GetAbility(self, AbilityEnum::ENEMY_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);

            auto& moveable = registry->get<MoveableActor>(self);
            const auto& combatable = registry->get<CombatableActor>(self);
            moveable.followTarget.emplace(combatable.target);
            auto& target = registry->get<MoveableActor>(combatable.target);

            auto syb = moveable.onDestinationReached.Subscribe(
                [this](entt::entity _entity) { onTargetReached(_entity); });
            auto syb1 = target.onPathChanged.Subscribe(
                [this, self](entt::entity _target) { onTargetPosUpdate(self, _target); });

            auto& state = registry->get<WavemobState>(self);
            state.ManageSubscription(std::move(syb1));
            state.ManageSubscription(std::move(syb));

            onTargetPosUpdate(self, combatable.target);
        }

        void OnExit(entt::entity self) override
        {
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
        }

        ~TargetOutOfRangeState() override = default;

        TargetOutOfRangeState(entt::registry* registry, Systems* _sys, WavemobStateMachine* _stateController)
            : State(registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class WavemobStateMachine::CombatState : public State
    {
        WavemobStateMachine* stateController;

        void onTargetDeath(entt::entity self, entt::entity target)
        {
        }

        [[nodiscard]] bool checkInCombat(entt::entity entity) const
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

        void OnEnter(entt::entity entity) override
        {
            auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
            registry->get<Ability>(abilityEntity).startCast.Publish(abilityEntity);
        }

        void OnExit(entt::entity entity) override
        {
            auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);
        }

        ~CombatState() override = default;

        CombatState(entt::registry* registry, Systems* _sys, WavemobStateMachine* _stateController)
            : State(registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class WavemobStateMachine::DyingState : public State
    {
        WavemobStateMachine* stateController;

        void destroyEntity(entt::entity self) const
        {
            auto& state = registry->get<WavemobState>(self);
            state.RemoveAllSubscriptions();
            registry->emplace<DeleteEntityComponent>(self);
        }

      public:
        void OnEnter(entt::entity self) override
        {
            LockState(self); // Target is dying, do not change state
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            combatable.dying = true;
            auto& bb = registry->get<Collideable>(self).worldBoundingBox;
            sys->navigationGridSystem->MarkSquareAreaOccupied(bb, false);
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
            auto& state = registry->get<WavemobState>(self);
            auto syb =
                animation.onAnimationEnd.Subscribe([this](entt::entity _entity) { destroyEntity(_entity); });
            state.ManageSubscription(std::move(syb));

            auto abilityEntity = sys->abilityFactory->GetAbility(self, AbilityEnum::ENEMY_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);

            sys->actorMovementSystem->CancelMovement(self);
        }

        void OnExit(entt::entity self) override
        {
            UnlockState(self);
        }

        ~DyingState() override = default;

        DyingState(entt::registry* registry, Systems* sys, WavemobStateMachine* _stateController)
            : State(registry, sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void WavemobStateMachine::Update()
    {
        for (const auto view = registry->view<WavemobState>(); const auto& entity : view)
        {
            const auto state = registry->get<WavemobState>(entity).GetCurrentStateEnum();
            GetStateFromEnum(state)->Update(entity);
        }
    }

    void WavemobStateMachine::Draw3D()
    {
        for (const auto view = registry->view<WavemobState>(); const auto& entity : view)
        {
            const auto state = registry->get<WavemobState>(entity).GetCurrentStateEnum();
            GetStateFromEnum(state)->Draw3D(entity);
        }
    }

    WavemobStateMachine::WavemobStateMachine(entt::registry* _registry, Systems* _sys) : StateMachine(_registry)
    {
        states[WavemobStateEnum::Default] = std::make_unique<DefaultState>(_registry, _sys, this);
        states[WavemobStateEnum::TargetOutOfRange] =
            std::make_unique<TargetOutOfRangeState>(_registry, _sys, this);
        states[WavemobStateEnum::Combat] = std::make_unique<CombatState>(_registry, _sys, this);
        states[WavemobStateEnum::Dying] = std::make_unique<DyingState>(_registry, _sys, this);
    }
} // namespace sage