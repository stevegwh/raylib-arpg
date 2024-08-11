#include "Ability.hpp"

#include <cassert>

#include "raylib.h"

namespace sage
{

    void Ability::IdleState::Update(entt::entity self)
    {
        ability->cooldownTimer.Update(GetFrameTime());
        if (ability->cooldownTimer.HasFinished() && ability->abilityData.repeatable)
        {
            ability->Init(self);
        }
    }

    // --------------------------------------------

    void Ability::CursorSelectState::enableCursor()
    {
        ability->spellCursor->Init(cursor->terrainCollision().point);
        ability->spellCursor->Enable(true);
        cursor->Disable();
        cursor->Hide();
    }

    void Ability::CursorSelectState::disableCursor()
    {
        cursor->Enable();
        cursor->Show();
        ability->spellCursor->Enable(false);
    }

    void Ability::CursorSelectState::Update(entt::entity self)
    {
        ability->spellCursor->Update(cursor->terrainCollision().point);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            // ability->confirm(self);
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
        ability->animationDelayTimer.Update(GetFrameTime());
        if (ability->animationDelayTimer.HasFinished())
        {
            // ability->OnAbilityExecute.publish(self);
            ability->Execute(self);
        }
    }

    void Ability::AwaitingExecutionState::OnEnter(entt::entity self)
    {
        ability->cooldownTimer.Start();
        ability->animationDelayTimer.Start();
    }

    // --------------------------------------------

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

    void Ability::initStates()
    {
        states[AbilityStateEnum::IDLE] = std::make_unique<IdleState>(this);
        states[AbilityStateEnum::CURSOR_SELECT] =
            std::make_unique<CursorSelectState>(this, cursor);
        states[AbilityStateEnum::AWAITING_EXECUTION] =
            std::make_unique<AwaitingExecutionState>(this);
        state = states[AbilityStateEnum::IDLE].get();
    }

    Ability::Ability(
        entt::registry* _registry, const AbilityData& _abilityData, Cursor* _cursor)
        : registry(_registry), abilityData(_abilityData), cursor(_cursor)
    {
        cooldownTimer.SetMaxTime(abilityData.cooldownDuration);
        animationDelayTimer.SetMaxTime(abilityData.animationDelay);
        initStates();
    }
} // namespace sage