// FountainEffect.hpp
#pragma once

#include "ParticleSystem.hpp"
#include "raylib.h"
#include <memory>

namespace sage
{
	struct FlameEffect : public ParticleSystem
	{
		Texture2D texCircle16{};
		Texture2D texCircle8{};

		~FlameEffect();
		explicit FlameEffect(Camera3D* cam);
	};	
}