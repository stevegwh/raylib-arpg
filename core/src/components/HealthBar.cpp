//
// Created by Steve Wheeler on 03/06/2024.
//

#include "HealthBar.hpp"

#include <cmath>

namespace sage
{
	void HealthBar::Decrement(int value)
	{
		damageTaken += value;
	}

	void HealthBar::Increment(int value)
	{
		damageTaken = 0;
	}

	void HealthBar::Update()
	{
		const float decayRate = 2.0f; // Adjust this value to control the speed of decay
		damageTaken *= exp(-decayRate * GetFrameTime());

		if (damageTaken < 0.1f) damageTaken = 0; // Reset to zero when it's very small

		BeginTextureMode(healthBarTexture);
		ClearBackground(BLANK);

		DrawRectangle(0, 0, 200, 20, healthBarBgColor);
		float healthPercentage = static_cast<float>(*hp) / 100.0f;
		float damageTakenPercentage = static_cast<float>(damageTaken) / 100.0f;
		int fillWidth = static_cast<int>(healthPercentage * 200);
		DrawRectangle(0, 0, fillWidth, 20, healthBarColor);
		DrawRectangle(fillWidth, 0, static_cast<int>(damageTakenPercentage * 200), 20, WHITE);
		DrawRectangleLines(0, 0, 200, 20, healthBarBorderColor);

		Vector2 textSize = MeasureTextEx(GetFontDefault(), TextFormat("HP: %03i", *hp), 20, 1);
		DrawTextEx(GetFontDefault(), TextFormat("HP: %03i", *hp),
				{10, healthBarTexture.texture.height - 30 - textSize.y}, 20, 1, GREEN);

		EndTextureMode();
	}

	HealthBar::HealthBar(int* _hp) : hp(_hp)
	{
		healthBarTexture = LoadRenderTexture(200, 50);
	}

	HealthBar::~HealthBar()
	{
		UnloadRenderTexture(healthBarTexture);
	}
}
