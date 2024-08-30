#include "AbilityStateMachine.hpp"

#include "abilities/AbilityData.hpp"
#include "AbilityFunctions.hpp"
#include "AbilityIndicator.hpp"
#include "AbilityResourceManager.hpp"
#include "components/Animation.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "TextureTerrainOverlay.hpp"
#include "Timer.hpp"
#include "vfx/VisualFX.hpp"

#include "raylib.h"
#include <cassert>
#include <iostream>

namespace sage
{
    class AbilityState
    {
      protected:
        entt::registry* registry;
        GameData* gameData;

      public:
        virtual void Update(entt::entity abilityEntity) {};
        virtual void Draw3D(entt::entity abilityEntity) {};
        virtual void OnEnter(entt::entity abilityEntity) {};
        virtual void OnExit(entt::entity abilityEntity) {};
        virtual ~AbilityState() {};
        AbilityState(entt::registry* _registry, GameData* _gameData) : registry(_registry), gameData(_gameData)
        {
        }
    };

    class AbilityStateMachine::IdleState : public AbilityState
    {

      public:
        entt::sigh<void(entt::entity)> onRestartTriggered;

        void Update(entt::entity abilityEntity) override
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.cooldownTimer.Update(GetFrameTime());
            if (ab.cooldownTimer.HasFinished() && ab.ad.base.repeatable)
            {
                onRestartTriggered.publish(
                    abilityEntity); // Was "caster". I think it was a mistake, so passed abilityEntity
            }
        }

        IdleState(entt::registry* _registry, GameData* _gameData) : AbilityState(_registry, _gameData)
        {
        }
    };

    // --------------------------------------------

    class AbilityStateMachine::CursorSelectState : public AbilityState
    {
        bool cursorActive = false; // Limits us to one cursor at once (I assume this is fine)

        void enableCursor(entt::entity abilityEntity)
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.abilityIndicator->Init(gameData->cursor->terrainCollision().point);
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
        entt::sigh<void(entt::entity)> onConfirm;
        void Update(entt::entity abilityEntity) override
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.abilityIndicator->Update(gameData->cursor->terrainCollision().point);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                onConfirm.publish(abilityEntity);
            }
        }

        void OnEnter(entt::entity abilityEntity) override
        {
            enableCursor(abilityEntity);
            cursorActive = true;
        }

        void OnExit(entt::entity abilityEntity) override
        {
            if (cursorActive)
            {
                disableCursor(abilityEntity);
                cursorActive = false;
            }
        }

        CursorSelectState(entt::registry* _registry, GameData* _gameData) : AbilityState(_registry, _gameData)
        {
        }
    };

    // --------------------------------------------

    class AbilityStateMachine::AwaitingExecutionState : public AbilityState
    {

        void signalExecute(entt::entity abilityEntity)
        {
            onExecute.publish(abilityEntity);
        }

      public:
        entt::sigh<void(entt::entity)> onExecute;

        void OnEnter(entt::entity abilityEntity) override
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.cooldownTimer.Start();
            ab.executionDelayTimer.Start();

            auto& ad = ab.ad;
            if (ad.base.behaviourPreHit == AbilityBehaviourPreHit::DETACHED_PROJECTILE)
            {
                GameObjectFactory::createProjectile(registry, ab.caster, abilityEntity, gameData);
                auto& moveable = registry->get<MoveableActor>(abilityEntity);
                entt::sink sink{moveable.onDestinationReached};
                sink.connect<&AwaitingExecutionState::signalExecute>(this);
            }
        }

        void Update(entt::entity abilityEntity) override
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            ab.executionDelayTimer.Update(GetFrameTime());
            auto& ad = ab.ad;

            if (ab.executionDelayTimer.HasFinished() &&
                ad.base.behaviourPreHit != AbilityBehaviourPreHit::DETACHED_PROJECTILE)
            {
                onExecute.publish(abilityEntity);
            }
        }

        AwaitingExecutionState(entt::registry* _registry, GameData* _gameData) : AbilityState(_registry, _gameData)

        {
        }
    };

    // ----------------------------

    void AbilityStateMachine::ChangeState(entt::entity abilityEntity, AbilityStateEnum newState)
    {
        assert(states.contains(newState));
        auto& ab = registry->get<Ability>(abilityEntity);
        states[ab.state]->OnExit(abilityEntity);
        ab.state = newState;
        states[ab.state]->OnEnter(abilityEntity);
    }

    void AbilityStateMachine::Cancel(entt::entity abilityEntity)
    {
        auto& ab = registry->get<Ability>(abilityEntity);
        if (ab.vfx && ab.vfx->active)
        {
            ab.vfx->active = false;
        }
        ab.cooldownTimer.Stop();
        ab.executionDelayTimer.Stop();
        ChangeState(abilityEntity, AbilityStateEnum::IDLE);
    }

    void AbilityStateMachine::Update()
    {
        auto view = registry->view<Ability>();
        for (auto abilityEntity : view)
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            if (!ab.IsActive()) continue;
            states[ab.state]->Update(abilityEntity);
            if (ab.vfx && ab.vfx->active)
            {
                ab.vfx->Update(GetFrameTime());
            }
        }
    }

    void AbilityStateMachine::Draw3D()
    {
        auto view = registry->view<Ability>();
        for (auto abilityEntity : view)
        {
            auto& ab = registry->get<Ability>(abilityEntity);
            if (!ab.IsActive()) continue;
            states[ab.state]->Draw3D(abilityEntity);
            if (ab.vfx && ab.vfx->active)
            {
                ab.vfx->Draw3D();
            }
        }
    }

    void AbilityStateMachine::Execute(entt::entity abilityEntity)
    {
        auto& ab = registry->get<Ability>(abilityEntity);
        auto& ad = ab.ad;

        if (ad.base.behaviourOnHit == AbilityBehaviourOnHit::HIT_TARGETED_UNIT)
        {
            auto& executeFunc = GetExecuteFunc<SingleTargetHit>(registry, ab.caster, abilityEntity, gameData);
            executeFunc.Execute();
        }
        else if (ad.base.behaviourOnHit == AbilityBehaviourOnHit::HIT_ALL_IN_RADIUS)
        {
            auto& executeFunc = GetExecuteFunc<HitAllInRadius>(registry, ab.caster, abilityEntity, gameData);
            executeFunc.Execute();
        }

        ChangeState(abilityEntity, AbilityStateEnum::IDLE);
    }

    void AbilityStateMachine::Confirm(entt::entity abilityEntity)
    {
        // Spawn target is either at cursor, at enemy, or at player
        // After spawned: Follow caster position, follow enemy position (maybe), follow the ability's detached
        // transform/collider (abilityEntity) and follow its position, or do nothing.
        auto& ab = registry->get<Ability>(abilityEntity);
        auto& ad = ab.ad;
        if (!registry->any_of<sgTransform>(abilityEntity))
        {
            registry->emplace<sgTransform>(abilityEntity, abilityEntity);
        }
        auto& trans = registry->get<sgTransform>(abilityEntity);

        if (ad.base.behaviourPreHit == AbilityBehaviourPreHit::DETACHED_PROJECTILE ||
            ad.base.behaviourPreHit == AbilityBehaviourPreHit::DETACHED_STATIONARY)
        {
            auto& casterPos = registry->get<sgTransform>(ab.caster).GetWorldPos();
            auto point = gameData->cursor->terrainCollision().point;
            if (Vector3Distance(point, casterPos) > ad.base.range)
            {
                return;
            }
        }

        auto& animation = registry->get<Animation>(ab.caster);
        animation.ChangeAnimationByParams(ad.animationParams);

        if (ab.vfx)
        {
            if (ad.base.spawnBehaviour == AbilitySpawnBehaviour::AT_CASTER)
            {
                auto& casterTrans = registry->get<sgTransform>(ab.caster);
                trans.SetPosition(casterTrans.GetWorldPos());
                trans.SetParent(&casterTrans);
                ab.vfx->InitSystem();
            }
            else if (ad.base.spawnBehaviour == AbilitySpawnBehaviour::AT_CURSOR)
            {
                trans.SetPosition(gameData->cursor->terrainCollision().point);
                ab.vfx->InitSystem();
            }
        }

        ChangeState(abilityEntity, AbilityStateEnum::AWAITING_EXECUTION);
    }

    // Determines if we need to display an indicator or not
    void AbilityStateMachine::Init(entt::entity abilityEntity)
    {

        auto& ab = registry->get<Ability>(abilityEntity);

        if (ab.ad.cursorBased) // Toggle indicator
        {
            if (ab.state == AbilityStateEnum::CURSOR_SELECT)
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
            Confirm(abilityEntity);
        }
    }

    AbilityStateMachine::~AbilityStateMachine()
    {
    }

    AbilityStateMachine::AbilityStateMachine(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {

        auto idleState = std::make_unique<IdleState>(_registry, _gameData);
        entt::sink onRestartTriggeredSink{idleState->onRestartTriggered};
        onRestartTriggeredSink.connect<&AbilityStateMachine::Init>(this);
        states[AbilityStateEnum::IDLE] = std::move(idleState);

        auto awaitingExecutionState = std::make_unique<AwaitingExecutionState>(_registry, _gameData);
        entt::sink onExecuteSink{awaitingExecutionState->onExecute};
        onExecuteSink.connect<&AbilityStateMachine::Execute>(this);
        states[AbilityStateEnum::AWAITING_EXECUTION] = std::move(awaitingExecutionState);

        auto cursorState = std::make_unique<CursorSelectState>(_registry, _gameData);
        entt::sink onConfirmSink{cursorState->onConfirm};
        onConfirmSink.connect<&AbilityStateMachine::Confirm>(this);
        states[AbilityStateEnum::CURSOR_SELECT] = std::move(cursorState);
    }

} // namespace sage