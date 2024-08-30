#include "AbilityStateMachine.hpp"

#include "abilities/AbilityData.hpp"
#include "AbilityFunctions.hpp"
#include "AbilityResourceManager.hpp"
#include "AbilityState.hpp"
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

    class AbilityStateMachine::IdleState : public AbilityState
    {

      public:
        entt::sigh<void(entt::entity)> onRestartTriggered;

        void Update() override
        {
            auto& ad = registry->get<AbilityData>(abilityEntity);
            cooldownTimer.Update(GetFrameTime());
            if (cooldownTimer.HasFinished() && ad.base.repeatable)
            {
                onRestartTriggered.publish(caster);
            }
        }

        IdleState(
            entt::registry* _registry,
            entt::entity _caster,
            entt::entity _abilityEntity,
            GameData* _gameData,
            Timer& cooldownTimer,
            Timer& executionDelayTimer)
            : AbilityState(_registry, _caster, _abilityEntity, _gameData, cooldownTimer, executionDelayTimer)
        {
        }
    };

    class AbilityStateMachine::AwaitingExecutionState : public AbilityState
    {
        VisualFX* vfx;

        void signalExecute()
        {
            onExecute.publish();
        }

      public:
        entt::sigh<void()> onExecute;

        void OnEnter() override
        {
            cooldownTimer.Start();
            executionDelayTimer.Start();

            auto& ad = registry->get<AbilityData>(abilityEntity);
            if (ad.base.behaviourPreHit == AbilityBehaviourPreHit::DETACHED_PROJECTILE)
            {
                GameObjectFactory::createProjectile(registry, caster, abilityEntity, gameData);
                auto& moveable = registry->get<MoveableActor>(abilityEntity);
                entt::sink sink{moveable.onDestinationReached};
                sink.connect<&AwaitingExecutionState::signalExecute>(this);
            }
        }

        void Update() override
        {
            executionDelayTimer.Update(GetFrameTime());
            auto& ad = registry->get<AbilityData>(abilityEntity);

            if (executionDelayTimer.HasFinished() &&
                ad.base.behaviourPreHit != AbilityBehaviourPreHit::DETACHED_PROJECTILE)
            {
                onExecute.publish();
            }
        }

        AwaitingExecutionState(
            entt::registry* _registry,
            entt::entity _caster,
            entt::entity _abilityEntity,
            GameData* _gameData,
            Timer& cooldownTimer,
            Timer& executionDelayTimer,
            VisualFX* _vfx)
            : AbilityState(_registry, _caster, _abilityEntity, _gameData, cooldownTimer, executionDelayTimer),
              vfx(_vfx)

        {
        }
    };

    // ----------------------------

    void AbilityStateMachine::ChangeState(AbilityStateEnum newState)
    {
        assert(states.contains(newState));
        state->OnExit();
        state = states[newState].get();
        state->OnEnter();
    }

    void AbilityStateMachine::ResetCooldown()
    {
        cooldownTimer.Reset();
    }

    bool AbilityStateMachine::IsActive()
    {
        return cooldownTimer.IsRunning();
    }

    float AbilityStateMachine::GetRemainingCooldownTime() const
    {
        return cooldownTimer.GetRemainingTime();
    }

    float AbilityStateMachine::GetCooldownDuration() const
    {
        return cooldownTimer.GetMaxTime();
    }

    bool AbilityStateMachine::CooldownReady() const
    {
        return cooldownTimer.HasFinished() || cooldownTimer.GetRemainingTime() <= 0;
    }

    void AbilityStateMachine::Cancel()
    {
        if (vfx && vfx->active)
        {
            vfx->active = false;
        }
        cooldownTimer.Stop();
        executionDelayTimer.Stop();
        ChangeState(AbilityStateEnum::IDLE);
    }

    void AbilityStateMachine::Update()
    {
        if (!IsActive()) return;
        state->Update();
        if (vfx && vfx->active)
        {
            vfx->Update(GetFrameTime());
        }
    }

    void AbilityStateMachine::Draw3D()
    {
        if (!IsActive()) return;
        state->Draw3D();
        if (vfx && vfx->active)
        {
            vfx->Draw3D();
        }
    }

    void AbilityStateMachine::Execute()
    {
        auto& ad = registry->get<AbilityData>(abilityEntity);

        if (ad.base.behaviourOnHit == AbilityBehaviourOnHit::HIT_TARGETED_UNIT)
        {
            auto& executeFunc = GetExecuteFunc<SingleTargetHit>(registry, caster, abilityEntity, gameData);
            executeFunc.Execute();
        }
        else if (ad.base.behaviourOnHit == AbilityBehaviourOnHit::HIT_ALL_IN_RADIUS)
        {
            auto& executeFunc = GetExecuteFunc<HitAllInRadius>(registry, caster, abilityEntity, gameData);
            executeFunc.Execute();
        }

        ChangeState(AbilityStateEnum::IDLE);
    }

    void AbilityStateMachine::Init()
    {
        auto& ad = registry->get<AbilityData>(abilityEntity);

        // Spawn target is either at cursor, at enemy, or at player
        // After spawned: Follow caster position, follow enemy position (maybe), follow the ability's detached
        // transform/collider (abilityEntity) and follow its position, or do nothing.

        if (!registry->any_of<sgTransform>(abilityEntity))
        {
            registry->emplace<sgTransform>(abilityEntity, abilityEntity);
        }
        auto& trans = registry->get<sgTransform>(abilityEntity);

        if (ad.base.behaviourPreHit == AbilityBehaviourPreHit::DETACHED_PROJECTILE)
        {
            auto& casterPos = registry->get<sgTransform>(caster).GetWorldPos();
            auto point = gameData->cursor->terrainCollision().point;
            if (Vector3Distance(point, casterPos) > ad.base.range)
            {
                return;
            }
        }

        auto& animation = registry->get<Animation>(caster);
        animation.ChangeAnimationByParams(ad.animationParams);

        if (vfx)
        {
            if (ad.base.spawnBehaviour == AbilitySpawnBehaviour::AT_CASTER)
            {
                auto& casterTrans = registry->get<sgTransform>(caster);
                vfx->InitSystem();
                trans.SetPosition(casterTrans.GetWorldPos());
                trans.SetParent(&casterTrans);
            }
            else if (ad.base.spawnBehaviour == AbilitySpawnBehaviour::AT_CURSOR)
            {
                vfx->InitSystem();
                trans.SetPosition(gameData->cursor->terrainCollision().point);
            }
        }

        ChangeState(AbilityStateEnum::AWAITING_EXECUTION);
    }

    AbilityStateMachine::~AbilityStateMachine()
    {
        registry->destroy(abilityEntity);
    }

    AbilityStateMachine::AbilityStateMachine(
        entt::registry* _registry, entt::entity _caster, entt::entity _abilityEntity, GameData* _gameData)
        : registry(_registry), caster(_caster), abilityEntity(_abilityEntity), gameData(_gameData)
    {

        // TODO: Would be great to find a way of pushing visual fx to the registry.
        // Atm it's difficult due to polymorphism
        auto& abilityData = registry->get<AbilityData>(_abilityEntity);
        vfx = AbilityResourceManager::GetInstance().GetVisualFX(abilityData.vfx, _abilityEntity, _gameData);

        cooldownTimer.SetMaxTime(abilityData.base.cooldownDuration);
        executionDelayTimer.SetMaxTime(abilityData.animationParams.animationDelay);

        auto idleState = std::make_unique<IdleState>(
            _registry, _caster, _abilityEntity, _gameData, cooldownTimer, executionDelayTimer);
        entt::sink onRestartTriggeredSink{idleState->onRestartTriggered};
        onRestartTriggeredSink.connect<&AbilityStateMachine::Init>(this);
        states[AbilityStateEnum::IDLE] = std::move(idleState);

        auto awaitingExecutionState = std::make_unique<AwaitingExecutionState>(
            _registry, _caster, _abilityEntity, _gameData, cooldownTimer, executionDelayTimer, vfx.get());
        entt::sink onExecuteSink{awaitingExecutionState->onExecute};
        onExecuteSink.connect<&AbilityStateMachine::Execute>(this);
        states[AbilityStateEnum::AWAITING_EXECUTION] = std::move(awaitingExecutionState);

        state = states[AbilityStateEnum::IDLE].get();
    }

} // namespace sage