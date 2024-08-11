#include "Ability.hpp"

#include <cassert>

#include "raylib.h"

namespace sage
{
    std::string getEnumName(
        AbilityStateEnum state,
        const std::unordered_map<AbilityStateEnum, std::unique_ptr<AbilityState>>&
            _states)
    {
        switch (state)
        {
        case AbilityStateEnum::IDLE:
            return "IDLE";
        case AbilityStateEnum::CURSOR_SELECT:
            return "CURSOR_SELECT";
        case AbilityStateEnum::AWAITING_EXECUTION:
            return "AWAITING_EXECUTION";
        default:
            return "UNKNOWN";
        }
    }

    // --------------------------------------------

    void Ability::IdleState::Update(entt::entity self)
    {
        cooldownTimer.Update(GetFrameTime());
        if (cooldownTimer.HasFinished() && repeatable)
        {
            onRestartTriggered.publish(self);
        }
    }

    // --------------------------------------------

    void Ability::CursorSelectState::enableCursor()
    {
        spellCursor->Init(cursor->terrainCollision().point);
        spellCursor->Enable(true);
        cursor->Disable();
        cursor->Hide();
    }

    void Ability::CursorSelectState::disableCursor()
    {
        cursor->Enable();
        cursor->Show();
        spellCursor->Enable(false);
    }

    void Ability::CursorSelectState::Update(entt::entity self)
    {
        spellCursor->Update(cursor->terrainCollision().point);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            onConfirm.publish(self);
        }
    }

    void Ability::CursorSelectState::toggleCursor(entt::entity self)
    {
        if (cursorActive)
        {
            disableCursor();
            cursorActive = false;
        }
        else
        {
            enableCursor();
            cursorActive = true;
        }
    }

    void Ability::CursorSelectState::OnExit(entt::entity self)
    {
        if (cursorActive)
        {
            disableCursor();
            cursorActive = false;
        }
    }

    void Ability::CursorSelectState::OnEnter(entt::entity self)
    {
        enableCursor();
        cursorActive = true;
    }

    // --------------------------------------------

    void Ability::AwaitingExecutionState::Update(entt::entity self)
    {
        animationDelayTimer.Update(GetFrameTime());
        if (animationDelayTimer.HasFinished())
        {
            onExecute.publish(self);
        }
    }

    void Ability::AwaitingExecutionState::OnEnter(entt::entity self)
    {
        cooldownTimer.Start();
        animationDelayTimer.Start();
    }

    // --------------------------------------------

    void Ability::ChangeState(entt::entity self, AbilityStateEnum newState)
    {
        assert(states.contains(newState));
        state->OnExit(self);
        state = states[newState].get();
        state->OnEnter(self);
    }

    void Ability::ResetCooldown()
    {
        cooldownTimer.Reset();
    }

    bool Ability::IsActive() const
    {
        return cooldownTimer.IsRunning();
    }

    float Ability::GetRemainingCooldownTime() const
    {
        return cooldownTimer.GetRemainingTime();
    }

    float Ability::GetCooldownDuration() const
    {
        return cooldownTimer.GetMaxTime();
    }

    bool Ability::CooldownReady() const
    {
        return cooldownTimer.HasFinished() || cooldownTimer.GetRemainingTime() <= 0;
    }

    void Ability::Cancel(entt::entity self)
    {
        cooldownTimer.Stop();
        animationDelayTimer.Stop();
        ChangeState(self, AbilityStateEnum::IDLE);
    }

    void Ability::Update(entt::entity self)
    {
        state->Update(self);
        if (vfx && vfx->active)
        {
            vfx->Update(GetFrameTime());
        }
    }

    void Ability::Draw3D(entt::entity self)
    {
        state->Draw3D(self);
        if (vfx && vfx->active)
        {
            vfx->Draw3D();
        }
    }

    void Ability::Init(entt::entity self)
    {
        Execute(self);
    }

    Ability::Ability(
        entt::registry* _registry, const AbilityData& _abilityData, Cursor* _cursor)
        : registry(_registry), abilityData(_abilityData), cursor(_cursor)
    {
        cooldownTimer.SetMaxTime(abilityData.cooldownDuration);
        animationDelayTimer.SetMaxTime(abilityData.animationDelay);

        auto idleState = std::make_unique<IdleState>(
            cooldownTimer, animationDelayTimer, _abilityData.repeatable);
        states[AbilityStateEnum::IDLE] = std::move(idleState);
        state = states[AbilityStateEnum::IDLE].get();
    }
} // namespace sage