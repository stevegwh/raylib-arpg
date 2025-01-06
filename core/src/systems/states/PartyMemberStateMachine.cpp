#include "PartyMemberStateMachine.hpp"

#include "components/Animation.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "GameData.hpp"
#include "StateMachines.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "raylib.h"

#include <cassert>
#include <format>

static constexpr int FOLLOW_DISTANCE = 15;

namespace sage
{
    class PartyMemberStateController::DefaultState : public StateMachine
    {
        PartyMemberStateController* stateController;

      public:
        void onLeaderMove(entt::entity self, entt::entity leader)
        {
            gameData->actorMovementSystem->CancelMovement(self);
            stateController->ChangeState<FollowingLeaderState, entt::entity>(
                self, PartyMemberStateEnum::FollowingLeader, leader);
        }

        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnStateExit(entt::entity entity) override
        {
        }

        ~DefaultState() override = default;

        DefaultState(entt::registry* _registry, GameData* _gameData, PartyMemberStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PartyMemberStateController::FollowingLeaderState final : public StateMachine
    {
        PartyMemberStateController* stateController;

        void onDestinationUnreachable(const entt::entity self, const Vector3 requestedPos) const
        {
            auto& moveable = registry->get<MoveableActor>(self);
            stateController->ChangeState<DestinationUnreachableState, entt::entity, Vector3, PartyMemberStateEnum>(
                self,
                PartyMemberStateEnum::DestinationUnreachable,
                moveable.followTarget->targetActor,
                requestedPos,
                PartyMemberStateEnum::FollowingLeader);
        }

        void onMovementCancelled(const entt::entity self) const
        {
            stateController->ChangeState(self, PartyMemberStateEnum::Default);
        }

        void onTargetPathChanged(const entt::entity self, const entt::entity target) const
        {
            const auto& trans = registry->get<sgTransform>(self);
            const auto& targetTrans = registry->get<sgTransform>(target);
            const auto& targetMoveable = registry->get<MoveableActor>(target);
            auto dest = targetMoveable.IsMoving() ? targetMoveable.GetDestination() : targetTrans.GetWorldPos();
            const auto dir = Vector3Normalize(Vector3Subtract(dest, trans.GetWorldPos()));
            dest = Vector3Subtract(dest, Vector3MultiplyByValue(dir, FOLLOW_DISTANCE));
            gameData->actorMovementSystem->PathfindToLocation(self, dest);
        }

        void onTargetReached(const entt::entity self) const
        {
            stateController->ChangeState(self, PartyMemberStateEnum::Default);
        }

      public:
        void Update(const entt::entity self) override
        {
            const auto& moveable = registry->get<MoveableActor>(self);
            const auto& transform = registry->get<sgTransform>(self);
            const auto& followTrans = registry->get<sgTransform>(moveable.followTarget->targetActor);
            const auto& followMoveable = registry->get<MoveableActor>(moveable.followTarget->targetActor);

            // If we are closer to our destination than the leader is, then wait.
            if (followMoveable.IsMoving() &&
                Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE >
                    Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
            {
                auto followTarget = moveable.followTarget->targetActor;
                stateController->ChangeState<WaitingForLeaderState, entt::entity>(
                    self, PartyMemberStateEnum::WaitingForLeader, followTarget);
            }
        }

        void OnStateEnter(const entt::entity self, entt::entity followTarget)
        {
            assert(followTarget != entt::null);
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.emplace(registry, self, followTarget);

            const auto target = moveable.followTarget->targetActor;

            auto cnx =
                moveable.onDestinationReached->Subscribe([this](entt::entity _self) { onTargetReached(_self); });
            auto cnx1 = moveable.followTarget->onTargetPathChanged->Subscribe(
                [this](entt::entity _self, entt::entity _target) { onTargetPathChanged(_self, _target); });
            auto cnx2 =
                moveable.onMovementCancel->Subscribe([this](entt::entity _self) { onMovementCancelled(_self); });
            auto cnx3 =
                moveable.onDestinationUnreachable->Subscribe([this](entt::entity _self, Vector3 requestedPos) {
                    onDestinationUnreachable(_self, requestedPos);
                });

            auto& state = registry->get<PartyMemberState>(self);
            state.ManageSubscription(std::move(cnx));
            state.ManageSubscription(std::move(cnx1));
            state.ManageSubscription(std::move(cnx2));
            state.ManageSubscription(std::move(cnx3));

            // entt::sink sink5{moveable.followTarget->onTargetMovementCancelled};
            // state.AddConnection(sink5.connect<&FollowingLeaderState::onMovementCancelled>(this));

            onTargetPathChanged(self, target);

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::RUN);
        }

        void OnStateExit(const entt::entity self) override
        {
            // TODO: We don't disconnect the connections here?
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
            gameData->actorMovementSystem->CancelMovement(self);
        }

        ~FollowingLeaderState() override = default;

        FollowingLeaderState(
            entt::registry* _registry, GameData* _gameData, PartyMemberStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };
    // ----------------------------

    class PartyMemberStateController::WaitingForLeaderState final : public StateMachine
    {
        PartyMemberStateController* stateController;
        void onMovementCancelled(const entt::entity self) const
        {
            stateController->ChangeState(self, PartyMemberStateEnum::Default);
        }

      public:
        void Update(const entt::entity self) override
        {
            if (self == gameData->controllableActorSystem->GetSelectedActor())
            {
                stateController->ChangeState(self, PartyMemberStateEnum::Default);
            }

            const auto& moveable = registry->get<MoveableActor>(self);
            const auto& transform = registry->get<sgTransform>(self);
            const auto& followTrans = registry->get<sgTransform>(moveable.followTarget->targetActor);
            const auto& followMoveable = registry->get<MoveableActor>(moveable.followTarget->targetActor);

            // Follow target is now closer to its destination than we are, so we can proceed.
            if (followMoveable.IsMoving() &&
                Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE <
                    Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
            {
                auto followTarget = moveable.followTarget->targetActor;
                stateController->ChangeState<FollowingLeaderState, entt::entity>(
                    self, PartyMemberStateEnum::FollowingLeader, followTarget);
            }
        }

        void OnStateEnter(const entt::entity self, entt::entity followTarget)
        {
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.emplace(registry, self, followTarget);

            auto cnx =
                moveable.onMovementCancel->Subscribe([this](entt::entity entity) { onMovementCancelled(entity); });
            auto cnx1 = gameData->controllableActorSystem->onSelectedActorChange->Subscribe(
                [this](entt::entity, entt::entity entity) { onMovementCancelled(entity); });

            auto& state = registry->get<PartyMemberState>(self);
            state.ManageSubscription(std::move(cnx));
            state.ManageSubscription(std::move(cnx1));

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnStateEnter(const entt::entity self) override
        {
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnStateExit(const entt::entity self) override
        {
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
        }

        ~WaitingForLeaderState() override = default;

        WaitingForLeaderState(
            entt::registry* _registry, GameData* _gameData, PartyMemberStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PartyMemberStateController::DestinationUnreachableState : public StateMachine
    {
        PartyMemberStateController* stateController;
        struct StateData
        {
            entt::entity followTarget{};
            Vector3 originalDestination{};
            PartyMemberStateEnum previousState{};
            double timeStart{};
            float threshold{};
            unsigned int tryCount{};
            unsigned int maxTries{};
        };
        std::unordered_map<entt::entity, StateData> data;

      public:
        void Update(entt::entity entity) override
        {
            auto& moveable = registry->get<MoveableActor>(entity);
            if (moveable.IsMoving()) return;

            auto& _data = data[entity];

            if (_data.tryCount >= _data.maxTries)
            {
                moveable.followTarget.reset();
                gameData->actorMovementSystem->CancelMovement(entity);
                stateController->ChangeState(entity, PartyMemberStateEnum::Default);
            }

            if (GetTime() >= data[entity].timeStart + data[entity].threshold)
            {
                ++_data.tryCount;
                _data.timeStart = GetTime();
                if (gameData->actorMovementSystem->TryPathfindToLocation(entity, _data.originalDestination))
                {
                    if (_data.previousState == PartyMemberStateEnum::FollowingLeader)
                    {
                        stateController->ChangeState<FollowingLeaderState, entt::entity>(
                            entity, _data.previousState, _data.followTarget);
                    }
                    else
                    {
                        stateController->ChangeState(entity, _data.previousState);
                    }
                }
                else
                {
                    // If the leader is too far, we could maybe follow a party member who is closer to the
                    // destination and also moving
                    // TODO: Bug. Weird bug that going to the other player's path can make us walk past the player
                    auto leaderPos = registry->get<sgTransform>(_data.followTarget).GetWorldPos();
                    if (gameData->actorMovementSystem->TryPathfindToLocation(entity, leaderPos))
                    {
                        _data.tryCount = 0;
                    }
                }
            }
        }

        void OnStateEnter(
            entt::entity entity,
            entt::entity followTarget,
            Vector3 originalDestination,
            PartyMemberStateEnum previousState)
        {
            assert(previousState == PartyMemberStateEnum::FollowingLeader);
            data[entity] = {followTarget, originalDestination, previousState, GetTime(), 1.5f, 0, 4};

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnStateExit(entt::entity self) override
        {
            data.erase(self);
        }

        ~DestinationUnreachableState() override = default;

        DestinationUnreachableState(
            entt::registry* _registry, GameData* _gameData, PartyMemberStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void PartyMemberStateController::Update()
    {
        for (const auto view = registry->view<PartyMemberState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PlayerState>(entity));
            const auto state = registry->get<PartyMemberState>(entity).GetCurrentState();
            GetSystem(state)->Update(entity);
        }
    }

    void PartyMemberStateController::Draw3D()
    {
        for (const auto view = registry->view<PartyMemberState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PlayerState>(entity));
            const auto state = registry->get<PartyMemberState>(entity).GetCurrentState();
            GetSystem(state)->Draw3D(entity);
        }
    }

    void PartyMemberStateController::onComponentAdded(entt::entity entity)
    {
        auto& state = registry->get<PartyMemberState>(entity);

        // Forward leader's movement to the state's onLeaderMovement
        const auto& leaderMoveable =
            registry->get<MoveableActor>(gameData->controllableActorSystem->GetSelectedActor());

        state.onLeaderMoveForwardCnx =
            leaderMoveable.onStartMovement->Subscribe([this, entity](entt::entity leader) {
                const auto& _state = registry->get<PartyMemberState>(entity);
                _state.onLeaderMove->Publish(entity, leader);
            });

        state.onLeaderMove->Subscribe([this](const entt::entity self, const entt::entity leader) {
            GetSystem<DefaultState>(PartyMemberStateEnum::Default)->onLeaderMove(self, leader);
        });

        ChangeState(entity, PartyMemberStateEnum::Default);
    }

    void PartyMemberStateController::onComponentRemoved(entt::entity entity)
    {
        auto& state = registry->get<PartyMemberState>(entity);
        if (state.onLeaderMoveForwardCnx)
        {
            state.onLeaderMoveForwardCnx->UnSubscribe();
        }
    }

    PartyMemberStateController::PartyMemberStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry), gameData(_gameData)
    {
        states[PartyMemberStateEnum::Default] = std::make_unique<DefaultState>(_registry, _gameData, this);
        states[PartyMemberStateEnum::FollowingLeader] =
            std::make_unique<FollowingLeaderState>(_registry, _gameData, this);
        states[PartyMemberStateEnum::WaitingForLeader] =
            std::make_unique<WaitingForLeaderState>(_registry, _gameData, this);
        states[PartyMemberStateEnum::DestinationUnreachable] =
            std::make_unique<DestinationUnreachableState>(_registry, _gameData, this);

        registry->on_construct<PartyMemberState>().connect<&PartyMemberStateController::onComponentAdded>(this);
        registry->on_destroy<PartyMemberState>().connect<&PartyMemberStateController::onComponentRemoved>(this);
    }
} // namespace sage