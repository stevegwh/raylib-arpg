//
// Created by Steve Wheeler on 23/07/2024.
//

#include "FlamePartSys.hpp"

#include "ResourceManager.hpp"

namespace sage
{

    FlamePartSys::FlamePartSys(Camera3D* cam) : ParticleSystem(cam)
    {
        texCircle16 = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_SPARKFLAME);

        EmitterConfig ecfg1;
        ecfg1.size = 2.0f;
        ecfg1.direction = Vector3{0, 1, 0};
        ecfg1.velocity = FloatRange{0.7, 0.73};
        ecfg1.directionAngle = FloatRange{-6, 6};
        ecfg1.velocityAngle = FloatRange{0, 0};
        ecfg1.offset = FloatRange{0, 0};
        ecfg1.originAcceleration = FloatRange{0, 0};
        ecfg1.burst = IntRange{0, 0};
        ecfg1.capacity = 200;
        ecfg1.emissionRate = 200;
        ecfg1.origin = Vector3{0, 0, 0};
        ecfg1.externalAcceleration = Vector3{0, 0.85, 0};
        ecfg1.startColor = Color{255, 100, 0, 255};
        ecfg1.endColor = Color{255, 150, 150, 70};
        ecfg1.age = FloatRange{0.4, 0.7};
        ecfg1.blendMode = BLEND_ADDITIVE;
        ecfg1.texture = texCircle16;
        ecfg1.particle_Deactivator = Particle_DeactivatorAge;

        auto emitterFountain1 = std::make_unique<Emitter>(ecfg1);
        Register(std::move(emitterFountain1));

        //        ecfg1.size = 0.5f;
        //        ecfg1.velocity = FloatRange{0.4, 0.43};
        //        ecfg1.capacity = 600;
        //        ecfg1.emissionRate = 200;
        //        auto emitterFountain2 = std::make_unique<Emitter>(ecfg1);
        //        Register(std::move(emitterFountain2));

        Start();
    }
} // namespace sage