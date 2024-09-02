//
// Created by Steve Wheeler on 23/07/2024.
//

#include "FlamePartSys.hpp"

#include "ResourceManager.hpp"

namespace sage
{

    FlamePartSys::~FlamePartSys()
    {
        UnloadTexture(texCircle16);
    }

    FlamePartSys::FlamePartSys(Camera3D* cam) : ParticleSystem(cam)
    {
        // Image imgCircle16 = GenImageGradientRadialTrans(16, 16, 0.3f, WHITE, BLACK);
        // texCircle16 = LoadTextureFromImage(imgCircle16);
        // Image imgCircle8 = GenImageGradientRadialTrans(8, 8, 0.5f, WHITE, BLACK);
        // texCircle8 = LoadTextureFromImage(imgCircle8);

        texCircle16 = LoadTextureFromImage(ResourceManager::GetInstance().ImageLoad("resources/textures/spark_flame.png"));

        // UnloadImage(imgCircle8);
        // UnloadImage(imgCircle16);

        EmitterConfig ecfg1;
        ecfg1.size = 1.0f;
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
        ecfg1.externalAcceleration = Vector3{0, 0.85, 0};
        ecfg1.startColor = Color{255, 100, 0, 255};
        ecfg1.endColor = Color{255, 150, 150, 70};
        ecfg1.age = FloatRange{0, 1.0};
        ecfg1.blendMode = BLEND_ADDITIVE;
        ecfg1.texture = texCircle16;
        ecfg1.particle_Deactivator = Particle_DeactivatorAge;

        auto emitterFountain1 = std::make_unique<Emitter>(ecfg1);
        Register(std::move(emitterFountain1));

        Start();
    }
} // namespace sage