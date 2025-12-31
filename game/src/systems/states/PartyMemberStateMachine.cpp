#include "PartyMemberStateMachine.hpp"

#include "StateMachines.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "engine/components/Animation.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/systems/ActorMovementSystem.hpp"

#include "raylib.h"

#include <cassert>
#include <format>

static constexpr int FOLLOW_DISTANCE = 15;

namespace lq
{
    // Necessary data for when the following party member cannot reach their target destination
    struct DestinationUnreachableData
    {
        Vector3 originalDestination{};
        double timeStart{};
        unsigned int tryCount = 0;
        const float retryTimeThreshold = 1.5;
        const unsigned int maxTries = 10;
    };

    class PartyMemberStateMachine::DefaultState final : public sage::State
    {
        Systems* sys;
        PartyMemberStateMachine* stateController;

      public:
        void onLeaderMove(entt::entity self) const
        {
            sys->actorMovementSystem->CancelMovement(self);
            stateController->ChangeState(self, PartyMemberStateEnum::FollowingLeader);
        }

        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity) override
        {
            auto& animation = registry->get<sage::Animation>(entity);
            animation.ChangeAnimationByEnum(sage::AnimationEnum::IDLE);
        }

        void OnExit(entt::entity entity) override
        {
        }

        ~DefaultState() override = default;

        DefaultState(entt::registry* _registry, Systems* _sys, PartyMemberStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PartyMemberStateMachine::FollowingLeaderState final : public sage::State
    {
        Systems* sys;
        PartyMemberStateMachine* stateController;

        void onDestinationUnreachable(const entt::entity self, const Vector3 requestedPos) const
        {
            auto& moveable = registry->get<sage::MoveableActor>(self);
            registry->emplace<DestinationUnreachableData>(
                self, DestinationUnreachableData{.originalDestination = requestedPos, .timeStart = GetTime()});
            stateController->ChangeState(self, PartyMemberStateEnum::DestinationUnreachable);
        }

        void onMovementCancelled(const entt::entity self) const
        {
            stateController->ChangeState(self, PartyMemberStateEnum::Default);
        }

        void onTargetPathChanged(const entt::entity self, const entt::entity target) const
        {
            const auto& trans = registry->get<sage::sgTransform>(self);
            const auto& targetTrans = registry->get<sage::sgTransform>(target);
            const auto& targetMoveable = registry->get<sage::MoveableActor>(target);
            auto dest = targetMoveable.IsMoving() ? targetMoveable.GetDestination() : targetTrans.GetWorldPos();
            const auto dir = Vector3Normalize(Vector3Subtract(dest, trans.GetWorldPos()));
            dest = Vector3Subtract(dest, sage::Vector3MultiplyByValue(dir, FOLLOW_DISTANCE));
            sys->actorMovementSystem->PathfindToLocation(self, dest, true);
        }

        void onTargetReached(const entt::entity self) const
        {
            stateController->ChangeState(self, PartyMemberStateEnum::Default);
        }

      public:
        void Update(const entt::entity self) override
        {
            const auto& moveable = registry->get<sage::MoveableActor>(self);
            const auto& transform = registry->get<sage::sgTransform>(self);
            const auto& followTrans = registry->get<sage::sgTransform>(moveable.actorTarget.value());
            const auto& followMoveable = registry->get<sage::MoveableActor>(moveable.actorTarget.value());

            // If we are closer to our destination than the leader is, then wait.
            if (followMoveable.IsMoving() &&
                Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE >
                    Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
            {
                stateController->ChangeState(self, PartyMemberStateEnum::WaitingForLeader);
            }
        }

        void OnEnter(const entt::entity self, entt::entity actorTarget)
        {
            assert(actorTarget != entt::null);
            auto& animation = registry->get<sage::Animation>(self);
            animation.ChangeAnimationByEnum(sage::AnimationEnum::RUN);

            auto& moveable = registry->get<sage::MoveableActor>(self);
            moveable.actorTarget.emplace(actorTarget);

            auto& target = registry->get<sage::MoveableActor>(moveable.actorTarget.value());

            auto sub = moveable.onDestinationReached.Subscribe(
                [this](const entt::entity _self) { onTargetReached(_self); });
            auto sub1 = target.onPathChanged.Subscribe(
                [this, self](entt::entity _target) { onTargetPathChanged(self, _target); });
            auto sub2 = moveable.onMovementCancel.Subscribe(
                [this](const entt::entity _self) { onMovementCancelled(_self); });
            auto sub3 = moveable.onDestinationUnreachable.Subscribe(
                [this](const entt::entity _self, const Vector3 requestedPos) {
                    onDestinationUnreachable(_self, requestedPos);
                });

            auto& state = registry->get<PartyMemberState>(self);
            state.ManageSubscription(std::move(sub));
            state.ManageSubscription(std::move(sub1));
            state.ManageSubscription(std::move(sub2));
            state.ManageSubscription(std::move(sub3));

            // entt::sink sink5{moveable.actorTarget->onTargetMovementCancelled};
            // state.AddConnection(sink5.connect<&FollowingLeaderState::onMovementCancelled>(this));

            onTargetPathChanged(self, actorTarget);
        }

        void OnEnter(entt::entity) override
        {
            // Error if this overload of OnEnter is called.
            assert(0);
        }

        void OnExit(const entt::entity self) override
        {
            // TODO: We don't disconnect the subscriptions here?
            auto& moveable = registry->get<sage::MoveableActor>(self);
            moveable.actorTarget.reset();
            sys->actorMovementSystem->CancelMovement(self);
        }

        ~FollowingLeaderState() override = default;

        FollowingLeaderState(entt::registry* _registry, Systems* _sys, PartyMemberStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };
    // ----------------------------

    class PartyMemberStateMachine::WaitingForLeaderState final : public sage::State
    {
        Systems* sys;
        PartyMemberStateMachine* stateController;

        void onMovementCancelled(const entt::entity self) const
        {
            stateController->ChangeState(self, PartyMemberStateEnum::Default);
        }

      public:
        void Update(const entt::entity self) override
        {
            if (self == sys->cursor->GetSelectedActor())
            {
                stateController->ChangeState(self, PartyMemberStateEnum::Default);
            }

            const auto& moveable = registry->get<sage::MoveableActor>(self);
            const auto& transform = registry->get<sage::sgTransform>(self);
            const auto& followTrans = registry->get<sage::sgTransform>(moveable.actorTarget.value());
            const auto& followMoveable = registry->get<sage::MoveableActor>(moveable.actorTarget.value());

            // Follow target is now closer to its destination than we are, so we can proceed.
            if (followMoveable.IsMoving() &&
                Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE <
                    Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
            {
                stateController->ChangeState(self, PartyMemberStateEnum::FollowingLeader);
            }
        }

        void OnEnter(const entt::entity self) override
        {
            auto& moveable = registry->get<sage::MoveableActor>(self);
            assert(moveable.actorTarget.has_value());
            auto sub =
                moveable.onMovementCancel.Subscribe([this](entt::entity entity) { onMovementCancelled(entity); });
            auto sub1 = sys->cursor->onSelectedActorChange.Subscribe(
                [this](entt::entity, entt::entity entity) { onMovementCancelled(entity); });

            auto& state = registry->get<PartyMemberState>(self);
            state.ManageSubscription(std::move(sub));
            state.ManageSubscription(std::move(sub1));

            auto& animation = registry->get<sage::Animation>(self);
            animation.ChangeAnimationByEnum(sage::AnimationEnum::IDLE);
        }

        void OnExit(const entt::entity self) override
        {
            auto& moveable = registry->get<sage::MoveableActor>(self);
            moveable.actorTarget.reset();
        }

        ~WaitingForLeaderState() override = default;

        WaitingForLeaderState(entt::registry* _registry, Systems* _sys, PartyMemberStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PartyMemberStateMachine::DestinationUnreachableState final : public sage::State
    {
        Systems* sys;
        PartyMemberStateMachine* stateController;

      public:
        void Update(entt::entity self) override
        {
            auto& moveable = registry->get<sage::MoveableActor>(self);
            if (moveable.IsMoving()) return;
            auto& data = registry->get<DestinationUnreachableData>(self);

            if (data.tryCount >= data.maxTries)
            {
                // Give up trying.
                moveable.actorTarget.reset();
                sys->actorMovementSystem->CancelMovement(self);
                stateController->ChangeState(self, PartyMemberStateEnum::Default);
                return;
            }

            if (GetTime() >= data.timeStart + data.retryTimeThreshold)
            {
                ++data.tryCount;
                data.timeStart = GetTime();
                if (sys->actorMovementSystem->TryPathfindToLocation(self, data.originalDestination, true))
                {
                    stateController->ChangeState(self, PartyMemberStateEnum::FollowingLeader);
                }
                else
                {
                    // If the leader is too far, we could maybe follow a party member who is closer to the
                    // destination and also moving
                    // TODO: Bug. Weird bug that going to the other player's path can make us walk past the player
                    const auto& target = registry->get<sage::MoveableActor>(self).actorTarget;
                    assert(target.has_value());
                    auto leaderPos = registry->get<sage::sgTransform>(target.value()).GetWorldPos();
                    if (sys->actorMovementSystem->TryPathfindToLocation(self, leaderPos, true))
                    {
                        data.tryCount = 0;
                    }
                }
            }
        }

        void OnEnter(const entt::entity self) override
        {
            assert(registry->any_of<DestinationUnreachableData>(self));
            auto& animation{registry->get<sage::Animation>(self)};
            animation.ChangeAnimationByEnum(sage::AnimationEnum::IDLE);
        }

        void OnExit(const entt::entity self) override
        {
            registry->remove<DestinationUnreachableData>(self);
        }

        ~DestinationUnreachableState() override = default;

        DestinationUnreachableState(
            entt::registry* _registry, Systems* _sys, PartyMemberStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void PartyMemberStateMachine::Update()
    {
        for (const auto view = registry->view<PartyMemberState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PlayerState>(entity));
            const auto stateEnum = registry->get<PartyMemberState>(entity).GetCurrentStateEnum();
            GetStateFromEnum(stateEnum)->Update(entity);
        }
    }

    void PartyMemberStateMachine::Draw3D()
    {
        for (const auto view = registry->view<PartyMemberState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PlayerState>(entity));
            const auto stateEnum = registry->get<PartyMemberState>(entity).GetCurrentStateEnum();
            GetStateFromEnum(stateEnum)->Draw3D(entity);
        }
    }

    void PartyMemberStateMachine::onComponentAdded(entt::entity self)
    {
        auto& state{registry->get<PartyMemberState>(self)};
        const auto& leaderMoveable{registry->get<sage::MoveableActor>(sys->cursor->GetSelectedActor())};
        auto& moveable = registry->get<sage::MoveableActor>(self);
        moveable.actorTarget.emplace(sys->cursor->GetSelectedActor());
        auto& target = registry->get<sage::MoveableActor>(moveable.actorTarget.value());
        target.onStartMovement.Subscribe([this, self](const entt::entity) {
            static_cast<DefaultState*>(GetStateFromEnum(PartyMemberStateEnum::Default))->onLeaderMove(self);
        });
        ChangeState(self, PartyMemberStateEnum::Default);
    }

    void PartyMemberStateMachine::onComponentRemoved(const entt::entity self) const
    {
    }

    PartyMemberStateMachine::PartyMemberStateMachine(entt::registry* _registry, Systems* _sys)
        : StateMachine(_registry), sys(_sys)
    {
        states[PartyMemberStateEnum::Default] = std::make_unique<DefaultState>(_registry, _sys, this);
        states[PartyMemberStateEnum::FollowingLeader] =
            std::make_unique<FollowingLeaderState>(_registry, _sys, this);
        states[PartyMemberStateEnum::WaitingForLeader] =
            std::make_unique<WaitingForLeaderState>(_registry, _sys, this);
        states[PartyMemberStateEnum::DestinationUnreachable] =
            std::make_unique<DestinationUnreachableState>(_registry, _sys, this);

        registry->on_construct<PartyMemberState>().connect<&PartyMemberStateMachine::onComponentAdded>(this);
        registry->on_destroy<PartyMemberState>().connect<&PartyMemberStateMachine::onComponentRemoved>(this);
    }
} // namespace lq