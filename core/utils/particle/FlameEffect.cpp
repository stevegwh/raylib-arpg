//
// Created by Steve Wheeler on 23/07/2024.
//

#include "FlameEffect.hpp"

FlameEffect::~FlameEffect()
{
	UnloadTexture(texCircle8);
	UnloadTexture(texCircle16);
}

FlameEffect::FlameEffect(Camera3D* cam)
		: ParticleSystem(cam)
{
	Image imgCircle16 = GenImageGradientRadial(16, 16, 0.3f, WHITE, BLACK);
	texCircle16 = LoadTextureFromImage(imgCircle16);
	Image imgCircle8 = GenImageGradientRadial(8, 8, 0.5f, WHITE, BLACK);
	texCircle8 = LoadTextureFromImage(imgCircle8);
	UnloadImage(imgCircle8);
	UnloadImage(imgCircle16);

	EmitterConfig ecfg;
	ecfg.size =  1.0f;
	ecfg.direction = Vector3{ 0, 1, 0 };
	ecfg.velocity = FloatRange{ 0.7, 0.73 };
	ecfg.directionAngle = FloatRange{ -6, 6 };
	ecfg.velocityAngle = FloatRange{ 0, 0 };
	ecfg.offset = FloatRange{ 0, 0 };
	ecfg.originAcceleration = FloatRange{ 0, 0 };
	ecfg.burst = IntRange{ 0, 0 };
	ecfg.capacity = 600;
	ecfg.emissionRate = 200;
	ecfg.origin = Vector3{ 0, 0, 0 };
	ecfg.externalAcceleration = Vector3{ 0, 0.85, 0 };
	ecfg.startColor = Color{ 255, 100, 0, 255 };
	ecfg.endColor = Color{ 255, 150, 150, 70 };
	ecfg.age = FloatRange{ 0, 0.5 };
	ecfg.blendMode = BLEND_ADDITIVE;
	ecfg.texture = texCircle16;
	ecfg.particle_Deactivator = Particle_DeactivatorAge;

	auto emitter = std::make_unique<Emitter>(ecfg);
	Register(std::move(emitter));

	ecfg.directionAngle = FloatRange{ -1.5, 1.5 };
	ecfg.velocity = FloatRange{ 0.8, 0.85 };
	ecfg.texture = texCircle8;
	auto emitter2 = std::make_unique<Emitter>(ecfg);
	Register(std::move(emitter2));

	Start();

}
