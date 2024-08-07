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
    enum class AbilityState
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    static constexpr AbilityData _abilityData{
        .cooldownDuration = COOLDOWN,
        .range = 5,
        .baseDamage = DAMAGE,
        .element = AttackElement::FIRE};

    void RainOfFireAbility::EnableCursor()
    {
        spellCursor->Init(cursor->terrainCollision().point);
        spellCursor->Enable(true);
        cursor->Disable();
        cursor->Hide();
    }

    void RainOfFireAbility::DisableCursor()
    {
        cursor->Enable();
        cursor->Show();
        spellCursor->Enable(false);
    }

    void RainOfFireAbility::Init(entt::entity self)
    {
        if (state == AbilityState::CURSOR_SELECT)
        {
            // Cancel cursor
            DisableCursor();
            state = AbilityState::IDLE;
            return;
        }

        EnableCursor();
        state = AbilityState::CURSOR_SELECT;
    }

    void RainOfFireAbility::Execute(entt::entity self)
    {
        vfx->InitSystem(cursor->collision().point);
        Hit360AroundPoint(
            registry, self, abilityData, cursor->collision().point, whirlwindRadius);
        state = AbilityState::IDLE;
    }

    void RainOfFireAbility::Confirm(entt::entity self)
    {
        // std::cout << "Rain of fire ability used \n";

        cooldownTimer.Start();

        animationDelayTimer.Start();
        state = AbilityState::AWAITING_EXECUTION;

        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);

        DisableCursor();
    }

    void RainOfFireAbility::Draw3D(entt::entity self)
    {
        if (vfx->active)
        {
            vfx->Draw3D();
        }

        Ability::Draw3D(self);
    }

    void RainOfFireAbility::Update(entt::entity self)
    {

        cooldownTimer.Update(GetFrameTime());

        if (vfx->active)
        {
            vfx->Update(GetFrameTime());
        }

        if (state == AbilityState::CURSOR_SELECT)
        {
            spellCursor->Update(cursor->terrainCollision().point);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                Confirm(self);
            }
            return;
        }

        animationDelayTimer.Update(GetFrameTime());
        if (state == AbilityState::AWAITING_EXECUTION &&
            animationDelayTimer.HasFinished())
        {
            Execute(self);
        }
    }

    RainOfFireAbility::RainOfFireAbility(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        NavigationGridSystem* _navigationGridSystem)
        : Ability(_registry, _abilityData), cursor(_cursor), state(AbilityState::IDLE)
    {
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
