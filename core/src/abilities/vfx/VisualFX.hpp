#pragma once

#include "Camera.hpp"
#include "raylib.h"

namespace sage
{
    class VisualFX
    {
      protected:
        Camera* camera;

      public:
        bool active = false;
        virtual ~VisualFX() = default;
        virtual void InitSystem(const Vector3& _target) = 0;
        virtual void Update(float dt) = 0;
        virtual void Draw3D() const = 0;
        virtual void SetOrigin(const Vector3& origin) {};
        VisualFX(const VisualFX&) = delete;
        explicit VisualFX(Camera* _camera) : camera(_camera)
        {
        }
    };
} // namespace sage