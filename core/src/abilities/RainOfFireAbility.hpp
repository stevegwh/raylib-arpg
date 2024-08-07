#pragma once

#include "Ability.hpp"

#include "particle/RainOfFireVFX.hpp"
#include "TextureTerrainOverlay.hpp"

#include <memory>

namespace sage
{
    class NavigationGridSystem;
    class Cursor;
    class Camera;

    struct RainOfFireAbility : public Ability
    {
        bool shouldCast = false;
        Timer animationDelayTimer{};
        std::unique_ptr<RainOfFireVFX> vfx;
        Cursor* cursor;
        std::unique_ptr<TextureTerrainOverlay> spellCursor;
        float whirlwindRadius = 50.0f;
        void Init(entt::entity self) override;
        void Execute(entt::entity self) override;
        void Draw3D(entt::entity self) override;
        void Update(entt::entity self) override;
        void Confirm(entt::entity self);
        ~RainOfFireAbility() override = default;
        RainOfFireAbility(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            NavigationGridSystem* _navigationGridSystem);
    };
} // namespace sage