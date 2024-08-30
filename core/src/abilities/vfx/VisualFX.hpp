#pragma once

#include "raylib.h"

namespace sage
{
    class GameData;
    struct sgTransform;

    class VisualFX
    {
      protected:
        GameData* gameData;
        sgTransform* transform;

      public:
        bool active = false;
        virtual ~VisualFX() = default;
        virtual void InitSystem() = 0;
        virtual void Update(float dt) = 0;
        virtual void Draw3D() const = 0;
        VisualFX(const VisualFX&) = delete;
        explicit VisualFX(GameData* _gameData, sgTransform* _transform)
            : gameData(_gameData), transform(_transform)
        {
        }
    };
} // namespace sage