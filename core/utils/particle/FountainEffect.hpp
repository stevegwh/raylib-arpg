// FountainEffect.hpp
#pragma once

#include "ParticleSystem.hpp"
#include "raylib.h"
#include <memory>

namespace sage
{
	struct FountainEffect : public ParticleSystem
	{
		Texture2D texCircle16{};
		Texture2D texCircle8{};

		~FountainEffect();
		explicit FountainEffect(Camera3D* cam);

	};
}
