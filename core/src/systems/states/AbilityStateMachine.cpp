#include "AbilityStateMachine.hpp"

#include "abilities/AbilityData.hpp"
#include "abilities/AbilityFunctions.hpp"
#include "abilities/AbilityIndicator.hpp"
#include "abilities/vfx/VisualFX.hpp"
#include "AbilityFactory.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "TextureTerrainOverlay.hpp"
#include "Timer.hpp"

#include "raylib.h"
#include <cassert>
#include <iostream>

namespace sage
{
    class AbilityStateController::IdleState : public StateMachine
    {

      public:
        Event<entt::entity> onRestartTriggered;

        void Update(entt::entity abilityEntity) override
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.cooldownTimer.Update(GetFrameTime());
            if (ab.cooldownTimer.HasFinished() &&
                ab.ad.base.HasOptionalBehaviour(AbilityBehaviourOptional::REPEAT_AUTO))
            {
                onRestartTriggered.Publish(abilityEntity);
            }
        }

        IdleState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // --------------------------------------------

    class AbilityStateController::CursorSelectState : public StateMachine
    {
        bool cursorActive = false; // Limits us to one cursor at once (I assume this is fine)

        void enableCursor(entt::entity abilityEntity)
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.abilityIndicator->Init(gameData->cursor->getFirstNaviCollision().point);
            ab.abilityIndicator->Enable(true);
            gameData->cursor->Disable();
            gameData->cursor->Hide();
        }

        void disableCursor(entt::entity abilityEntity)
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            gameData->cursor->Enable();
            gameData->cursor->Show();
            ab.abilityIndicator->Enable(false);
        }

        void toggleCursor(entt::entity abilityEntity)
        {
            if (cursorActive)
            {
                disableCursor(abilityEntity);
                cursorActive = false;
            }
            else
            {
                enableCursor(abilityEntity);
                cursorActive = true;
            }
        }

      public:
        Event<entt::entity> onConfirm;
        void Update(entt::entity abilityEntity) override
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.abilityIndicator->Update(gameData->cursor->getFirstNaviCollision().point);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                onConfirm.Publish(abilityEntity);
            }
        }

        void OnStateEnter(entt::entity abilityEntity) override
        {
            enableCursor(abilityEntity);
            cursorActive = true;
        }

        void OnStateExit(entt::entity abilityEntity) override
        {
            if (cursorActive)
            {
                disableCursor(abilityEntity);
                cursorActive = false;
            }
        }

        CursorSelectState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // --------------------------------------------

    // TODO: I think this should be split into two states depending on whether its detached or not
    // Or maybe if it has a cast time or not...
    class AbilityStateController::AwaitingExecutionState : public StateMachine
    {

        void signalExecute(entt::entity abilityEntity)
        {
            onExecute.Publish(abilityEntity);
        }

      public:
        Event<entt::entity> onExecute;

        void OnStateEnter(entt::entity abilityEntity) override
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.cooldownTimer.Start();
            ab.castTimer.Start();

            auto& ad = ab.ad;
            if (ad.base.HasBehaviour(AbilityBehaviour::MOVEMENT_PROJECTILE))
            {
                createProjectile(registry, ab.caster, abilityEntity, gameData);
                auto& moveable = registry->get<MoveableActor>(abilityEntity);
                moveable.onDestinationReached.Subscribe([this](entt::entity _entity) { signalExecute(_entity); });
            }
        }

        void Update(entt::entity abilityEntity) override
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.castTimer.Update(GetFrameTime());
            auto& ad = ab.ad;

            // "executionDelayTimer" should just be a cast timer. Therefore, below should check for cast time
            // behaviour
            if (ab.castTimer.HasFinished() &&
                !ad.base.HasBehaviour(AbilityBehaviour::CAST_REGULAR)) // Might be FOLLOW_NONE
            {
                onExecute.Publish(abilityEntity);
            }
        }

        AwaitingExecutionState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)

        {
        }
    };

    // ----------------------------

    void AbilityStateController::cancelCast(entt::entity abilityEntity)
    {
        auto& ab = registry->get<Ability>(abilityEntity);
        if (ab.vfx && ab.vfx->active)
        {
            ab.vfx->active = false;
        }
        ab.cooldownTimer.Stop();
        ab.castTimer.Stop();
        ChangeState(abilityEntity, AbilityStateEnum::IDLE);
    }

    void AbilityStateController::executeAbility(entt::entity abilityEntity)
    {
        auto& ab = registry->get<Ability>(abilityEntity);
        auto& ad = ab.ad;

        if (ad.base.HasBehaviour(AbilityBehaviour::ATTACK_TARGET))
        {
            auto target = registry->get<CombatableActor>(ab.caster).target;
            HitSingleTarget(registry, ab.caster, abilityEntity, target);
        }
        else if (ad.base.HasBehaviour(AbilityBehaviour::ATTACK_AOE_POINT))
        {
            Vector3 targetPos{};

            if (ad.base.HasBehaviour(AbilityBehaviour::FOLLOW_NONE))
            {
                targetPos = registry->get<sgTransform>(abilityEntity).GetWorldPos();
            }
            else if (ad.base.HasBehaviour(AbilityBehaviour::FOLLOW_CASTER))
            {
                targetPos = registry->get<sgTransform>(ab.caster).GetWorldPos();
            }
            AOEAtPoint(registry, ab.caster, abilityEntity, targetPos, ad.base.radius);
        }

        ChangeState(abilityEntity, AbilityStateEnum::IDLE);
    }

    bool AbilityStateController::checkRange(entt::entity abilityEntity)
    {
        // TODO: Should account for more possiblities with flags here.
        auto& ab = registry->get<Ability>(abilityEntity);
        auto& ad = ab.ad;

        if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CURSOR))
        {
            auto& casterPos = registry->get<sgTransform>(ab.caster).GetWorldPos();
            auto point = gameData->cursor->getFirstNaviCollision().point;
            if (Vector3Distance(point, casterPos) > ad.base.range)
            {
                std::cout << "Out of range. \n";
                ab.castFailed.Publish(abilityEntity, AbilityCastFail::OUT_OF_RANGE);
                return false;
            }
        }
        return true;
    }

    void AbilityStateController::spawnAbility(entt::entity abilityEntity)
    {
        auto& ab = registry->get<Ability>(abilityEntity);
        auto& ad = ab.ad;

        if (!checkRange(abilityEntity)) return;

        auto& animation = registry->get<Animation>(ab.caster);
        animation.ChangeAnimationByParams(ad.animationParams);

        if (ab.vfx)
        {
            auto& trans = registry->get<sgTransform>(abilityEntity);
            if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CASTER))
            {
                auto& casterTrans = registry->get<sgTransform>(ab.caster);
                auto& casterBB = registry->get<Collideable>(ab.caster).worldBoundingBox;
                float heightOffset = Vector3Subtract(casterBB.max, casterBB.min).y;

                if (ad.base.HasBehaviour(AbilityBehaviour::FOLLOW_CASTER))
                {
                    // TODO: Can this be set in the ability's constructor?
                    // Then we can just say "if ability doesnt follow caster, then set its position"
                    if (trans.GetParent() != &casterTrans)
                    {
                        trans.SetParent(&casterTrans);
                        trans.SetLocalPos({0, heightOffset, 0});
                        trans.SetLocalRot(Vector3Zero());
                    }
                }
                else
                {

                    Vector3 pos = {casterTrans.GetWorldPos().x, heightOffset, casterTrans.GetWorldPos().z};
                    trans.SetPosition(pos);
                    trans.SetRotation(casterTrans.GetWorldRot());
                }

                ab.vfx->InitSystem();
            }
            else if (ad.base.HasBehaviour(AbilityBehaviour::SPAWN_AT_CURSOR))
            {
                trans.SetPosition(gameData->cursor->getFirstNaviCollision().point);
                ab.vfx->InitSystem();
            }
        }

        ChangeState(abilityEntity, AbilityStateEnum::AWAITING_EXECUTION);
    }

    // Determines if we need to display an indicator or not
    void AbilityStateController::startCast(entt::entity abilityEntity)
    {

        auto& ab = registry->get<Ability>(abilityEntity);

        if (ab.ad.base.HasOptionalBehaviour(AbilityBehaviourOptional::INDICATOR))
        {
            auto state = registry->get<AbilityState>(abilityEntity).GetCurrentState();
            if (state == AbilityStateEnum::CURSOR_SELECT)
            {
                ChangeState(abilityEntity, AbilityStateEnum::IDLE);
            }
            else
            {
                ChangeState(abilityEntity, AbilityStateEnum::CURSOR_SELECT);
            }
        }
        else
        {
            spawnAbility(abilityEntity);
        }
    }

    void AbilityStateController::Update()
    {
        auto view = registry->view<AbilityState, Ability>();
        for (auto abilityEntity : view)
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            auto state = registry->get<AbilityState>(abilityEntity).GetCurrentState();
            if (!(ab.IsActive() || state == AbilityStateEnum::CURSOR_SELECT))
            {
                continue;
            }

            states.at(state)->Update(abilityEntity);
            if (ab.vfx && ab.vfx->active)
            {
                ab.vfx->Update(GetFrameTime());
            }
        }
    }

    void AbilityStateController::Draw3D()
    {
        auto view = registry->view<AbilityState, Ability>();
        for (auto abilityEntity : view)
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            auto state = registry->get<AbilityState>(abilityEntity).GetCurrentState();
            if (!(ab.IsActive() || state == AbilityStateEnum::CURSOR_SELECT))
            {
                continue;
            }
            states.at(state)->Draw3D(abilityEntity);
            if (ab.vfx && ab.vfx->active)
            {
                ab.vfx->Draw3D();
            }
        }
    }

    void AbilityStateController::onComponentAdded(entt::entity addedEntity)
    {
        auto& ability = registry->get<Ability>(addedEntity);
        ability.onStartCastCnx = ability.startCast.Subscribe([this](entt::entity _entity) { startCast(_entity); });

        ability.onCancelCastCnx =
            ability.cancelCast.Subscribe([this](entt::entity _entity) { cancelCast(_entity); });
    }

    void AbilityStateController::onComponentRemoved(entt::entity addedEntity) const
    {
        // TODO: Could add the check for "FOLLOW_CASTER" here instead.
        const auto& ability = registry->get<Ability>(addedEntity);
        ability.onStartCastCnx->UnSubscribe();
        ability.onStartCastCnx->UnSubscribe();
    }

    AbilityStateController::AbilityStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry), gameData(_gameData)
    {
        registry->on_construct<Ability>().connect<&AbilityStateController::onComponentAdded>(this);
        registry->on_destroy<Ability>().connect<&AbilityStateController::onComponentRemoved>(this);

        auto idleState = std::make_unique<IdleState>(_registry, _gameData);
        idleState->onRestartTriggered.Subscribe([this](entt::entity _entity) { startCast(_entity); });
        states[AbilityStateEnum::IDLE] = std::move(idleState);

        auto awaitingExecutionState = std::make_unique<AwaitingExecutionState>(_registry, _gameData);
        awaitingExecutionState->onExecute.Subscribe([this](entt::entity _entity) { executeAbility(_entity); });
        states[AbilityStateEnum::AWAITING_EXECUTION] = std::move(awaitingExecutionState);

        auto cursorState = std::make_unique<CursorSelectState>(_registry, _gameData);
        cursorState->onConfirm.Subscribe([this](entt::entity _entity) { spawnAbility(_entity); });
        states[AbilityStateEnum::CURSOR_SELECT] = std::move(cursorState);
    }

} // namespace sage