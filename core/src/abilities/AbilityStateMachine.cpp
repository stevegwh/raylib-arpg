#include "AbilityStateMachine.hpp"

#include "abilities/AbilityData.hpp"
#include "AbilityFunctions.hpp"
#include "AbilityResourceManager.hpp"
#include "AbilityState.hpp"
#include "components/Animation.hpp"
#include "components/MovableActor.hpp"
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
            Timer& animationDelayTimer)
            : AbilityState(_registry, _caster, _abilityEntity, _gameData, cooldownTimer, animationDelayTimer)
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
            animationDelayTimer.Start();

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
            animationDelayTimer.Update(GetFrameTime()); // Execute delay timer?

            // Depending on vfx behaviour, update its position here
            auto& ad = registry->get<AbilityData>(abilityEntity);

            // TODO: With a transform hierarchy, below wouldn't be necessary. About time to allow transform
            // parents/children?
            if (vfx)
            {
                if (ad.base.behaviourPreHit == AbilityBehaviourPreHit::FOLLOW_CASTER)
                {
                    auto casterPos = registry->get<sgTransform>(caster).position();
                    auto& abilityTrans = registry->get<sgTransform>(abilityEntity);
                    abilityTrans.SetPosition(casterPos, abilityEntity);
                    GridSquare gs;
                    gameData->navigationGridSystem->WorldToGridSpace(abilityTrans.position(), gs);

                    vfx->SetOrigin(
                        {abilityTrans.position().x,
                         gameData->navigationGridSystem->GetGridSquare(gs.row, gs.col)->terrainHeight + 3.0f,
                         abilityTrans.position().z});
                }
                else if (ad.base.behaviourPreHit == AbilityBehaviourPreHit::DETACHED_PROJECTILE)
                {
                    auto& abilityTrans = registry->get<sgTransform>(abilityEntity);
                    // Need some options for height
                    vfx->SetOrigin({abilityTrans.position().x, 5.0f, abilityTrans.position().z});
                }
            }
            // ----

            if (animationDelayTimer.HasFinished() &&
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
            Timer& animationDelayTimer,
            VisualFX* _vfx)
            : AbilityState(_registry, _caster, _abilityEntity, _gameData, cooldownTimer, animationDelayTimer),
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
        animationDelayTimer.Stop();
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
        auto& animation = registry->get<Animation>(caster);
        auto& ad = registry->get<AbilityData>(abilityEntity);
        animation.ChangeAnimationByParams(ad.animationParams);

        // Spawn target is either at cursor, at enemy, or at player
        // After spawned: Follow caster position, follow enemy position (maybe), follow the ability's detached
        // transform/collider (abilityEntity) and follow its position, or do nothing.

        if (!registry->any_of<sgTransform>(abilityEntity))
        {
            registry->emplace<sgTransform>(abilityEntity);
        }
        auto& trans = registry->get<sgTransform>(abilityEntity);

        if (vfx)
        {
            if (ad.base.spawnBehaviour == AbilitySpawnBehaviour::AT_CASTER)
            {
                auto casterPos = registry->get<sgTransform>(caster).position();
                vfx->InitSystem(casterPos);
                trans.SetPosition(casterPos, abilityEntity);
            }
            else if (ad.base.spawnBehaviour == AbilitySpawnBehaviour::AT_CURSOR)
            {
                vfx->InitSystem(gameData->cursor->terrainCollision().point);
                trans.SetPosition(gameData->cursor->terrainCollision().point, abilityEntity);
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
        vfx = AbilityResourceManager::GetInstance().GetVisualFX(abilityData.vfx, _gameData);

        cooldownTimer.SetMaxTime(abilityData.base.cooldownDuration);
        animationDelayTimer.SetMaxTime(abilityData.animationParams.animationDelay);

        auto idleState = std::make_unique<IdleState>(
            _registry, _caster, _abilityEntity, _gameData, cooldownTimer, animationDelayTimer);
        entt::sink onRestartTriggeredSink{idleState->onRestartTriggered};
        onRestartTriggeredSink.connect<&AbilityStateMachine::Init>(this);
        states[AbilityStateEnum::IDLE] = std::move(idleState);

        auto awaitingExecutionState = std::make_unique<AwaitingExecutionState>(
            _registry, _caster, _abilityEntity, _gameData, cooldownTimer, animationDelayTimer, vfx.get());
        entt::sink onExecuteSink{awaitingExecutionState->onExecute};
        onExecuteSink.connect<&AbilityStateMachine::Execute>(this);
        states[AbilityStateEnum::AWAITING_EXECUTION] = std::move(awaitingExecutionState);

        state = states[AbilityStateEnum::IDLE].get();
    }

} // namespace sage