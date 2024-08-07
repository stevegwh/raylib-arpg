//
// Created by Steve Wheeler on 21/07/2024.
//

#include "RainOfFireAbility.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "AbilityFunctions.hpp"

#include "raylib.h"

static constexpr float COOLDOWN = 3.0f;
static constexpr float WINDUP = 0.75f;
static constexpr int DAMAGE = 25;

namespace sage
{
    static constexpr AbilityData _abilityData{
        .element = AttackElement::FIRE,
        .cooldownDuration = COOLDOWN,
        .baseDamage = DAMAGE,
        .range = 5};

    void RainOfFireAbility::Init(entt::entity self)
    {
        if (GetRemainingCooldownTime() > 0)
        {
            std::cout << "Waiting for cooldown \n";
            return;
        }
        if (!spellCursor->active())
        {
            spellCursor->Init(cursor->terrainCollision().point);
            spellCursor->Enable(true);
            cursor->Disable();
            cursor->Hide();
            return;
        }
        else
        {
            cursor->Enable();
            cursor->Show();
            spellCursor->Enable(false); // Cancel move
        }
    }

    void RainOfFireAbility::Execute(entt::entity self)
    {
        vfx->InitSystem(cursor->collision().point);
        Hit360AroundPoint(
            registry, self, abilityData, cursor->collision().point, whirlwindRadius);
        active = false;
    }

    void RainOfFireAbility::Confirm(entt::entity self)
    {
        std::cout << "Rain of fire ability used \n";

        cooldownTimer.Start();
        windupTimer.Start();

        active = true;
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);
        spellCursor->Enable(false);
        cursor->Enable();
        cursor->Show();
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

        if (spellCursor->active())
        {
            spellCursor->Update(cursor->terrainCollision().point);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !active)
            {
                Confirm(self);
            }
            return;
        }

        windupTimer.Update(GetFrameTime());
        if (windupTimer.HasFinished() && active)
        {
            Execute(self);
        }
    }

    RainOfFireAbility::RainOfFireAbility(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        CollisionSystem* _collisionSystem,
        NavigationGridSystem* _navigationGridSystem,
        ControllableActorSystem* _controllableActorSystem)
        : Ability(_registry, _abilityData, _collisionSystem),
          cursor(_cursor),
          controllableActorSystem(_controllableActorSystem)
    {
        windupTimer.SetMaxTime(WINDUP);
        spellCursor = std::make_unique<TextureTerrainOverlay>(
            registry,
            _navigationGridSystem,
            "resources/textures/cursor/rainoffire_cursor.png",
            Color{255, 215, 0, 255},
            "resources/shaders/glsl330/bloom.fs");
        vfx = std::make_unique<RainOfFireVFX>(_camera->getRaylibCam());
    }
} // namespace sage
