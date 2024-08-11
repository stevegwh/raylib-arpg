#pragma once

#include "Ability.hpp"

#include <memory>

namespace sage
{
    class Camera;

    class CursorAbility : public Ability
    {
        float whirlwindRadius = 50.0f;

      protected:
        virtual void confirm(entt::entity self);
        CursorAbility(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            std::unique_ptr<TextureTerrainOverlay> _spellCursor,
            AbilityData _abilityData);

      public:
        void Init(entt::entity self) override;
        void Execute(entt::entity self) override;
        ~CursorAbility() override = default;
    };
} // namespace sage