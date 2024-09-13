//
// Created by Steve Wheeler on 23/07/2024.
//

#include "FountainPartSys.hpp"
#include "slib.hpp"

#include "ResourceManager.hpp"

namespace sage
{

    FountainPartSys::FountainPartSys(Camera3D* cam) : ParticleSystem(cam)
    {
        auto texCircle16 = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_IMGCIRCLE16);
        auto texCircle8 = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_IMGCIRCLE8);

        EmitterConfig ecfg1;
        ecfg1.size = 0.2f;
        ecfg1.direction = Vector3{0, 1, 0};
        ecfg1.velocity = FloatRange{0.7, 0.73};
        ecfg1.directionAngle = FloatRange{-6, 6};
        ecfg1.velocityAngle = FloatRange{0, 0};
        ecfg1.offset = FloatRange{0, 0};
        ecfg1.originAcceleration = FloatRange{0, 0};
        ecfg1.burst = IntRange{0, 0};
        ecfg1.capacity = 600;
        ecfg1.emissionRate = 200;
        ecfg1.origin = Vector3{0, 0, 0};
        ecfg1.externalAcceleration = Vector3{0, -0.30, 0};
        ecfg1.startColor = Color{0, 100, 150, 255};
        ecfg1.endColor = Color{0, 0, 255, 125};
        ecfg1.age = FloatRange{0, 6};
        ecfg1.blendMode = BLEND_ADDITIVE;
        ecfg1.texture = texCircle16;
        ecfg1.particle_Deactivator = Particle_DeactivatorAge;

        auto emitterFountain1 = std::make_unique<Emitter>(ecfg1);
        Register(std::move(emitterFountain1));

        ecfg1.directionAngle = FloatRange{-1.5, 1.5};
        ecfg1.velocity = FloatRange{0.8, 0.85};
        ecfg1.texture = texCircle8;
        auto emitterFountain2 = std::make_unique<Emitter>(ecfg1);
        Register(std::move(emitterFountain2));

        Start();
    }
} // namespace sage
