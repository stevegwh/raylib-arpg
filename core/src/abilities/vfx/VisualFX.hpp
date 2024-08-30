#pragma once

#include "raylib.h"

namespace sage
{
    class GameData;

    class VisualFX
    {
      protected:
        GameData* gameData;

      public:
        bool active = false;
        virtual ~VisualFX() = default;
        virtual void InitSystem(const Vector3& _target) = 0;
        virtual void Update(float dt) = 0;
        virtual void Draw3D() const = 0;
        virtual void SetOrigin(const Vector3& origin) {};
        VisualFX(const VisualFX&) = delete;
        explicit VisualFX(GameData* _gameData) : gameData(_gameData)
        {
        }
    };
} // namespace sage