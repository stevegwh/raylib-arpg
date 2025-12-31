//
// Created by Steve Wheeler on 23/07/2024.
//

#include "FlamePartSys.hpp"

#include "engine/ResourceManager.hpp"

namespace lq
{

    FlamePartSys::FlamePartSys(Camera3D* cam) : ParticleSystem(cam)
    {
        texCircle16 =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/particles/smoke_04.png");

        sage::EmitterConfig ecfg1;
        ecfg1.size = 9.0f;
        ecfg1.direction = Vector3{0, -1, 0};
        ecfg1.velocity = sage::FloatRange{-19.1, -17.1};
        ecfg1.directionAngle = sage::FloatRange{-10, -5};
        ecfg1.velocityAngle = sage::FloatRange{-10, 10};
        ecfg1.offset = sage::FloatRange{-25, -10};
        ecfg1.originAcceleration = sage::FloatRange{-10, -5};
        ecfg1.burst = sage::IntRange{0, 0};
        ecfg1.capacity = 400;
        ecfg1.emissionRate = 300;
        ecfg1.origin = Vector3{0, 0, 0};
        ecfg1.externalAcceleration = Vector3{0, -0.85, 0};
        ecfg1.startColor = Color{180, 100, 0, 255};
        ecfg1.endColor = Color{0, 0, 0, 255};
        ecfg1.age = sage::FloatRange{0, 0.8};
        ecfg1.blendMode = BLEND_ADDITIVE;
        ecfg1.texture = texCircle16;
        ecfg1.particle_Deactivator = sage::Particle_DeactivatorAge;

        auto emitterFountain1 = std::make_unique<sage::Emitter>(ecfg1);
        Register(std::move(emitterFountain1));

        Start();
    }
} // namespace lq