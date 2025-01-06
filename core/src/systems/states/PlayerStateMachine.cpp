#include "PlayerStateMachine.hpp"

#include "GameData.hpp"

#include "AbilityFactory.hpp"
#include "Camera.hpp"
#include "components/Ability.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/ControllableActor.hpp"
#include "components/DialogComponent.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "PartyMemberStateMachine.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"

#include "raylib.h"
#include "StateMachines.hpp"

#include <cassert>
#include <format>

static constexpr int FOLLOW_DISTANCE = 15;

namespace sage
{
    class PlayerStateController::DefaultState : public StateMachine
    {
        PlayerStateController* stateController;

        // If all below are persistent connections, maybe it would be better to move them to
        // the PlayerStateController? Having them here implies they are part of the state and will
        // disconnect on state change.

        void onFloorClick(const entt::entity self, entt::entity x) const
        {
            auto& state = registry->get<PlayerState>(self);
            // We're not allowed to change to the same state, so change to default and then back again
            if (state.GetCurrentState() == PlayerStateEnum::MovingToLocation)
            {
                stateController->ChangeState(self, PlayerStateEnum::Default);
            }
            stateController->ChangeState(self, PlayerStateEnum::MovingToLocation);
        }

        void onNPCLeftClick(entt::entity self, entt::entity target) const
        {
            if (!registry->any_of<DialogComponent>(target)) return;

            if (registry->any_of<MoveableActor>(target)) // Not all NPCs move
            {
                auto& moveable = registry->get<MoveableActor>(self);
                moveable.followTarget.emplace(registry, self, target);
            }
            stateController->ChangeState<MovingToTalkToNPCState, entt::entity>(
                self, PlayerStateEnum::MovingToTalkToNPC, target);
        }

        void onEnemyLeftClick(entt::entity self, entt::entity target) const
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = target;
            stateController->ChangeState(self, PlayerStateEnum::MovingToAttackEnemy);
        }

      public:
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

        DefaultState(entt::registry* _registry, GameData* _gameData, PlayerStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }

        friend class PlayerStateController;
    };

    // ----------------------------

    class PlayerStateController::MovingToLocationState : public StateMachine
    {
        PlayerStateController* stateController;

        void onMovementCancelled(entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnStateEnter(entt::entity self) override
        {
            gameData->actorMovementSystem->CancelMovement(self);
            gameData->actorMovementSystem->PathfindToLocation(self, gameData->cursor->getFirstCollision().point);
            auto& moveable = registry->get<MoveableActor>(self);
            auto& state = registry->get<PlayerState>(self);
            auto cnx = moveable.onDestinationReached->Subscribe(
                [this](entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(cnx));
            auto cnx1 = moveable.onMovementCancel->Subscribe(
                [this](entt::entity _entity) { onMovementCancelled(_entity); });
            state.ManageSubscription(std::move(cnx1));

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::RUN);
        }

        void OnStateExit(entt::entity self) override
        {
        }

        ~MovingToLocationState() override = default;

        MovingToLocationState(
            entt::registry* _registry, GameData* _gameData, PlayerStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::MovingToTalkToNPCState final : public StateMachine
    {
        PlayerStateController* stateController;
        void onMovementCancelled(const entt::entity self) const
        {
            auto& dialogComponent = registry->get<DialogComponent>(self);
            dialogComponent.dialogTarget = entt::null;
            auto& moveable = registry->get<MoveableActor>(self);
            moveable.followTarget.reset();
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(const entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::InDialog);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnStateEnter(entt::entity self, entt::entity target)
        {
            auto& moveable = registry->get<MoveableActor>(self);
            auto& playerDiag = registry->get<DialogComponent>(self);
            playerDiag.dialogTarget = target;
            const auto& pos = registry->get<DialogComponent>(playerDiag.dialogTarget).conversationPos;
            gameData->actorMovementSystem->PathfindToLocation(self, pos);

            auto& state = registry->get<PlayerState>(self);
            auto cnx = moveable.onDestinationReached->Subscribe(
                [this](entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(cnx));
            auto cnx1 = moveable.onMovementCancel->Subscribe(
                [this](entt::entity _entity) { onMovementCancelled(_entity); });
            state.ManageSubscription(std::move(cnx1));

            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::RUN);
        }

        void OnStateExit(entt::entity self) override
        {
        }

        ~MovingToTalkToNPCState() override = default;

        MovingToTalkToNPCState(
            entt::registry* _registry, GameData* _gameData, PlayerStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::DestinationUnreachableState : public StateMachine
    {
        PlayerStateController* stateController;
        struct StateData
        {
            Vector3 originalDestination{};
            PlayerStateEnum previousState{};
            double timeStart{};
            float threshold{};
            unsigned int tryCount{};
            unsigned int maxTries{};
        };
        std::unordered_map<entt::entity, StateData> data;

      public:
        void Update(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity, Vector3 originalDestination, PlayerStateEnum previousState)
        {
            data[entity] = {originalDestination, previousState, GetTime(), 1.5f, 0, 4};

            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }

        void OnStateExit(entt::entity self) override
        {
            data.erase(self);
        }

        ~DestinationUnreachableState() override = default;

        DestinationUnreachableState(
            entt::registry* _registry, GameData* _gameData, PlayerStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::InDialogState : public StateMachine
    {
        PlayerStateController* stateController;

      public:
        void OnStateEnter(entt::entity self) override
        {
            auto& playerDiag = registry->get<DialogComponent>(self);
            registry->get<Animation>(self).ChangeAnimationByEnum(AnimationEnum::TALK);
            if (registry->any_of<Animation>(playerDiag.dialogTarget))
            {
                registry->get<Animation>(playerDiag.dialogTarget).ChangeAnimationByEnum(AnimationEnum::TALK);
            }

            // Rotate to look at NPC
            auto& actorTrans = registry->get<sgTransform>(self);
            const auto& npcTrans = registry->get<sgTransform>(playerDiag.dialogTarget);
            Vector3 direction = Vector3Subtract(npcTrans.GetWorldPos(), actorTrans.GetWorldPos());
            direction = Vector3Normalize(direction);
            const float angle = atan2f(direction.x, direction.z);
            actorTrans.SetRotation({actorTrans.GetWorldRot().x, RAD2DEG * angle, actorTrans.GetWorldRot().z});

            gameData->dialogSystem->StartConversation(npcTrans, playerDiag.dialogTarget);
        }

        void OnStateExit(entt::entity self) override
        {
            auto& playerDiag = registry->get<DialogComponent>(self);
            if (registry->any_of<Animation>(playerDiag.dialogTarget))
            {
                registry->get<Animation>(playerDiag.dialogTarget).ChangeAnimationByEnum(AnimationEnum::IDLE);
            }
            playerDiag.dialogTarget = entt::null;

            // TODO: Bug: Doesn't change back to default on dialog end
        }

        ~InDialogState() override = default;

        InDialogState(entt::registry* _registry, GameData* _gameData, PlayerStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateController::MovingToAttackEnemyState : public StateMachine
    {
        PlayerStateController* stateController;
        void onAttackCancelled(entt::entity self, entt::entity) const
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Combat);
        }

      public:
        void OnStateEnter(entt::entity self) override
        {
            auto& animation = registry->get<Animation>(self);
            animation.ChangeAnimationByEnum(AnimationEnum::RUN);

            auto& moveableActor = registry->get<MoveableActor>(self);

            auto& state = registry->get<PlayerState>(self);
            auto cnx = moveableActor.onDestinationReached->Subscribe(
                [this](const entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(cnx));

            auto& combatable = registry->get<CombatableActor>(self);
            assert(combatable.target != entt::null);

            auto& controllable = registry->get<ControllableActor>(self);
            auto cnx1 = controllable.onFloorClick->Subscribe(
                [this](const entt::entity _self, entt::entity clicked) { onAttackCancelled(_self, clicked); });
            state.ManageSubscription(std::move(cnx1));

            const auto& enemyTrans = registry->get<sgTransform>(combatable.target);

            Vector3 playerPos = registry->get<sgTransform>(self).GetWorldPos();
            Vector3 enemyPos = enemyTrans.GetWorldPos();
            Vector3 direction = Vector3Subtract(enemyPos, playerPos);
            direction = Vector3Scale(Vector3Normalize(direction), combatable.attackRange);

            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            gameData->actorMovementSystem->PathfindToLocation(self, targetPos);
        }

        void OnStateExit(entt::entity self) override
        {
        }

        ~MovingToAttackEnemyState() override = default;

        MovingToAttackEnemyState(
            entt::registry* _registry, GameData* _gameData, PlayerStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    // TODO: Move to its own state machine
    class PlayerStateController::CombatState : public StateMachine
    {
        PlayerStateController* stateController;
        void onAttackCancelled(entt::entity self, entt::entity x)
        {
            // Both outcomes are the same
            onTargetDeath(self, entt::null);
        }

        void onTargetDeath(entt::entity self, entt::entity target)
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        bool checkInCombat(entt::entity entity)
        {
            // Might do more here later
            return true;
        }

      public:
        void Update(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);

            auto abilityEntity = gameData->abilityRegistry->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).startCast.publish(abilityEntity);

            auto& combatable = registry->get<CombatableActor>(entity);
            assert(combatable.target != entt::null);

            auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);
            combatable.onTargetDeathCnx =
                enemyCombatable.onDeath->Subscribe([entity, this](const entt::entity target) {
                    const auto& c = registry->get<CombatableActor>(entity);
                    c.onTargetDeath->Publish(entity, target);
                });

            auto& state = registry->get<PlayerState>(entity);
            auto cnx = combatable.onTargetDeath->Subscribe(
                [this](entt::entity self, entt::entity target) { onTargetDeath(self, target); });
            state.ManageSubscription(std::move(cnx));
        }

        void OnStateExit(entt::entity entity) override
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            combatable.onTargetDeathCnx->UnSubscribe();
            auto abilityEntity = gameData->abilityRegistry->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.publish(abilityEntity);
        }

        ~CombatState() override = default;
        CombatState(entt::registry* _registry, GameData* _gameData, PlayerStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void PlayerStateController::Update()
    {
        for (const auto view = registry->view<PlayerState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PartyMemberState>(entity));
            const auto state = registry->get<PlayerState>(entity).GetCurrentState();
            GetSystem(state)->Update(entity);
        }
    }

    void PlayerStateController::Draw3D()
    {
        for (const auto view = registry->view<PlayerState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PartyMemberState>(entity));
            const auto state = registry->get<PlayerState>(entity).GetCurrentState();
            GetSystem(state)->Draw3D(entity);
        }
    }

    void PlayerStateController::onComponentAdded(entt::entity entity)
    {
        // Cursor and controllable events are connected in ControllableActorSystem
        auto& controllable = registry->get<ControllableActor>(entity);
        controllable.onEnemyLeftClickCnx =
            controllable.onEnemyLeftClick->Subscribe([this](entt::entity self, entt::entity target) {
                GetSystem<DefaultState>(PlayerStateEnum::Default)->onEnemyLeftClick(self, target);
            });
        controllable.onNPCLeftClickCnx =
            controllable.onNPCLeftClick->Subscribe([this](entt::entity self, entt::entity target) {
                GetSystem<DefaultState>(PlayerStateEnum::Default)->onNPCLeftClick(self, target);
            });
        controllable.onFloorClickCnx =
            controllable.onFloorClick->Subscribe([this](entt::entity self, entt::entity target) {
                GetSystem<DefaultState>(PlayerStateEnum::Default)->onFloorClick(self, target);
            });
        // ----------------------------
    }

    void PlayerStateController::onComponentRemoved(entt::entity entity)
    {
        auto& controllable = registry->get<ControllableActor>(entity);
        controllable.onEnemyLeftClickCnx->UnSubscribe();
        controllable.onNPCLeftClickCnx->UnSubscribe();
        controllable.onFloorClickCnx->UnSubscribe();
    }

    PlayerStateController::PlayerStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry)
    {
        states[PlayerStateEnum::Default] = std::make_unique<DefaultState>(_registry, _gameData, this);
        states[PlayerStateEnum::MovingToAttackEnemy] =
            std::make_unique<MovingToAttackEnemyState>(_registry, _gameData, this);
        states[PlayerStateEnum::Combat] = std::make_unique<CombatState>(_registry, _gameData, this);
        states[PlayerStateEnum::MovingToTalkToNPC] =
            std::make_unique<MovingToTalkToNPCState>(_registry, _gameData, this);
        states[PlayerStateEnum::InDialog] = std::make_unique<InDialogState>(_registry, _gameData, this);
        states[PlayerStateEnum::MovingToLocation] =
            std::make_unique<MovingToLocationState>(_registry, _gameData, this);
        states[PlayerStateEnum::DestinationUnreachable] =
            std::make_unique<DestinationUnreachableState>(_registry, _gameData, this);

        registry->on_construct<PlayerState>().connect<&PlayerStateController::onComponentAdded>(this);
        registry->on_destroy<PlayerState>().connect<&PlayerStateController::onComponentRemoved>(this);
    }
} // namespace sage