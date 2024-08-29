#include "AbilityStateMachine.hpp"

#include "abilities/AbilityData.hpp"
#include "AbilityFunctions.hpp"
#include "AbilityResourceManager.hpp"
#include "AbilityState.hpp"
#include "components/Animation.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "raylib.h"
#include "TextureTerrainOverlay.hpp"
#include "Timer.hpp"
#include "vfx/VisualFX.hpp"
#include <cassert>

#include "components/sgTransform.hpp"
#include "systems/ControllableActorSystem.hpp"
#include <iostream>

namespace sage
{

    class AbilityStateMachine::IdleState : public AbilityState
    {
        const bool repeatable;

      public:
        entt::sigh<void(entt::entity)> onRestartTriggered;
        void Update() override
        {
            cooldownTimer.Update(GetFrameTime());
            if (cooldownTimer.HasFinished() && repeatable)
            {
                onRestartTriggered.publish(self);
            }
        }

        IdleState(entt::entity _self, Timer& coolDownTimer, Timer& animationDelayTimer, bool repeatable)
            : AbilityState(_self, coolDownTimer, animationDelayTimer), repeatable(repeatable)
        {
        }
    };

    class AbilityStateMachine::AwaitingExecutionState : public AbilityState
    {
      public:
        entt::sigh<void(entt::entity)> onExecute;
        void OnEnter() override
        {
            cooldownTimer.Start();
            animationDelayTimer.Start();
        }

        void Update() override
        {
            animationDelayTimer.Update(GetFrameTime());
            if (animationDelayTimer.HasFinished())
            {
                onExecute.publish(self);
            }
        }

        AwaitingExecutionState(entt::entity _self, Timer& coolDownTimer, Timer& animationDelayTimer)
            : AbilityState(_self, coolDownTimer, animationDelayTimer)
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
            // assert(true == false);
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
        abilityData.executeFunc->Execute(registry, self, abilityData);

        ChangeState(AbilityStateEnum::IDLE);
    }

    void AbilityStateMachine::Init()
    {
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByParams(abilityData.animationParams);
        // if (ad.vfx.behaviour == AbilityData::VFXBehaviour::SpawnAtPlayer)
        // vfx->InitSystem(data->controllableActorSystem->GetControlledActor()); // TODO: store caster's entity ID
        // in AbilityData

        if (vfx)
        {
            auto casterPos =
                registry->get<sgTransform>(gameData->controllableActorSystem->GetControlledActor()).position();
            vfx->InitSystem(casterPos);
        }
        ChangeState(AbilityStateEnum::AWAITING_EXECUTION);
    }

    AbilityStateMachine::~AbilityStateMachine()
    {
    }

    // TODO: I feel the "self" arguments are redundant, might as well just make it a member (as entities hold full
    // instances of abilities)

    AbilityStateMachine::AbilityStateMachine(
        entt::registry* registry, entt::entity _self, const AbilityData& _abilityData, GameData* _gameData)
        : registry(registry),
          self(_self),
          abilityData(_abilityData),
          gameData(_gameData),
          vfx(AbilityResourceManager::GetInstance().GetVisualFX(abilityData.vfx, _gameData))
    {
        cooldownTimer.SetMaxTime(abilityData.base.cooldownDuration);
        animationDelayTimer.SetMaxTime(abilityData.animationParams.animationDelay);

        auto idleState =
            std::make_unique<IdleState>(_self, cooldownTimer, animationDelayTimer, _abilityData.base.repeatable);
        entt::sink onRestartTriggeredSink{idleState->onRestartTriggered};
        onRestartTriggeredSink.connect<&AbilityStateMachine::Init>(this);
        states[AbilityStateEnum::IDLE] = std::move(idleState);

        auto awaitingExecutionState =
            std::make_unique<AwaitingExecutionState>(_self, cooldownTimer, animationDelayTimer);
        entt::sink onExecuteSink{awaitingExecutionState->onExecute};
        onExecuteSink.connect<&AbilityStateMachine::Execute>(this);
        states[AbilityStateEnum::AWAITING_EXECUTION] = std::move(awaitingExecutionState);

        state = states[AbilityStateEnum::IDLE].get();
    }

} // namespace sage