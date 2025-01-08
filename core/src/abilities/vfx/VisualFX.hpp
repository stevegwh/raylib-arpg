#pragma once

#include "raylib.h"
#include "entt/entt.hpp"

namespace sage
{
    class Systems;
    struct sgTransform;
    struct Ability;

    class VisualFX
    {
      protected:
        Ability* ability;
        Systems* sys;

      public:
        bool active = false;
        virtual ~VisualFX() = default;
        virtual void InitSystem() = 0;
        virtual void Update(float dt) = 0;
        virtual void Draw3D() const = 0;
        VisualFX(const VisualFX&) = delete;
        explicit VisualFX(Systems* _sys, Ability* _ability) : sys(_sys), ability(_ability)
        {
        }
    };
} // namespace sage