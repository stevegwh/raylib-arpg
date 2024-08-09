//
// Created by Steve Wheeler on 21/07/2024.
//

#include "RainOfFireAbility.hpp"

#include "Camera.hpp"
#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"

#include "AbilityFunctions.hpp"

#include "raylib.h"

static constexpr float COOLDOWN = 3.0f;
static constexpr float WINDUP = 0.75f;
static constexpr int DAMAGE = 25;

namespace sage
{

    void RainOfFireAbility::IdleState::Update(entt::entity self)
    {
        ability->cooldownTimer.Update(GetFrameTime());
        if (ability->vfx->active)
        {
            ability->vfx->Update(GetFrameTime());
        }
    }

    void RainOfFireAbility::IdleState::Draw3D(entt::entity self)
    {
        if (ability->vfx->active)
        {
            ability->vfx->Draw3D();
        }
    }

    void RainOfFireAbility::CursorSelectState::enableCursor()
    {
        ability->spellCursor->Init(ability->cursor->terrainCollision().point);
        ability->spellCursor->Enable(true);
        ability->cursor->Disable();
        ability->cursor->Hide();
    }

    void RainOfFireAbility::CursorSelectState::disableCursor()
    {
        ability->cursor->Enable();
        ability->cursor->Show();
        ability->spellCursor->Enable(false);
    }

    void RainOfFireAbility::CursorSelectState::Update(entt::entity self)
    {
        ability->spellCursor->Update(ability->cursor->terrainCollision().point);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            ability->confirm(self);
        }
    }

    void RainOfFireAbility::CursorSelectState::toggleCursor(entt::entity self)
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

    void RainOfFireAbility::CursorSelectState::OnExit(entt::entity self)
    {
        if (cursorActive)
        {
            disableCursor();
            cursorActive = false;
        }
    }

    void RainOfFireAbility::CursorSelectState::OnEnter(entt::entity self)
    {
        enableCursor();
        cursorActive = true;
    }

    void RainOfFireAbility::AwaitingExecutionState::Update(entt::entity self)
    {
        ability->animationDelayTimer.Update(GetFrameTime());
        if (ability->animationDelayTimer.HasFinished())
        {
            ability->Execute(self);
        }
    }

    void RainOfFireAbility::AwaitingExecutionState::OnEnter(entt::entity self)
    {
        ability->cooldownTimer.Start();

        ability->animationDelayTimer.Start();

        auto& animation = ability->registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);
    }

    static constexpr AbilityData _abilityData{
        .cooldownDuration = COOLDOWN,
        .range = 5,
        .baseDamage = DAMAGE,
        .element = AttackElement::FIRE};

    void RainOfFireAbility::Init(entt::entity self)
    {
        if (state == states[AbilityState::CURSOR_SELECT].get())
        {
            ChangeState(self, AbilityState::IDLE);
        }
        else
        {
            ChangeState(self, AbilityState::CURSOR_SELECT);
        }
    }

    void RainOfFireAbility::Execute(entt::entity self)
    {
        vfx->InitSystem(cursor->collision().point);
        Hit360AroundPoint(
            registry, self, abilityData, cursor->collision().point, whirlwindRadius);

        ChangeState(self, AbilityState::IDLE);
    }

    void RainOfFireAbility::confirm(entt::entity self)
    {
        ChangeState(self, AbilityState::AWAITING_EXECUTION);
    }

    void RainOfFireAbility::Draw3D(entt::entity self)
    {
        state->Draw3D(self);
        Ability::Draw3D(self);
    }

    void RainOfFireAbility::Update(entt::entity self)
    {
        state->Update(self);
    }

    RainOfFireAbility::RainOfFireAbility(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        NavigationGridSystem* _navigationGridSystem)
        : Ability(_registry, _abilityData), cursor(_cursor)
    {
        states[AbilityState::IDLE] = std::make_unique<IdleState>(this);
        states[AbilityState::CURSOR_SELECT] = std::make_unique<CursorSelectState>(this);
        states[AbilityState::AWAITING_EXECUTION] =
            std::make_unique<AwaitingExecutionState>(this);
        state = states[AbilityState::IDLE].get();

        animationDelayTimer.SetMaxTime(WINDUP);
        spellCursor = std::make_unique<TextureTerrainOverlay>(
            registry,
            _navigationGridSystem,
            "resources/textures/cursor/rainoffire_cursor.png",
            Color{255, 215, 0, 255},
            "resources/shaders/glsl330/bloom.fs");
        vfx = std::make_unique<RainOfFireVFX>(_camera->getRaylibCam());
    }
} // namespace sage
