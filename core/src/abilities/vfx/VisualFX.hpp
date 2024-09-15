#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    struct sgTransform;
    struct Ability;

    class VisualFX
    {
      protected:
        Ability* ability;
        GameData* gameData;

      public:
        bool active = false;
        virtual ~VisualFX() = default;
        virtual void InitSystem() = 0;
        virtual void Update(float dt) = 0;
        virtual void Draw3D() const = 0;
        VisualFX(const VisualFX&) = delete;
        explicit VisualFX(GameData* _gameData, Ability* _ability) : gameData(_gameData), ability(_ability)
        {
        }
    };
} // namespace sage