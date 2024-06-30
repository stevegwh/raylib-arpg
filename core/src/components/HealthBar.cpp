//
// Created by Steve Wheeler on 03/06/2024.
//

#include "HealthBar.hpp"

namespace sage
{
	void HealthBar::Decrement(entt::entity entity, int value)
	{
		hp -= value;
		onHealthUpdate.publish(entity);
		if (value <= 0)
		{
			onHealthIsZero.publish(entity);
		}
	}

	void HealthBar::Increment(entt::entity entity, int value)
	{
		hp += value;
		onHealthUpdate.publish(entity);
	}

	void HealthBar::UpdateTexture() const
	{
		BeginTextureMode(healthBarTexture);
		ClearBackground(BLANK);

		DrawRectangle(0, 0, 200, 20, healthBarBgColor);
		float healthPercentage = static_cast<float>(hp) / 100.0f;
		int fillWidth = static_cast<int>(healthPercentage * 200);
		DrawRectangle(0, 0, fillWidth, 20, healthBarColor);
		DrawRectangleLines(0, 0, 200, 20, healthBarBorderColor);

		Vector2 textSize = MeasureTextEx(GetFontDefault(), TextFormat("HP: %03i", hp), 20, 1);
		DrawTextEx(GetFontDefault(), TextFormat("HP: %03i", hp),
		           {10, healthBarTexture.texture.height - 30 - textSize.y}, 20, 1, GREEN);

		EndTextureMode();
	}

	HealthBar::HealthBar()
	{
		healthBarTexture = LoadRenderTexture(200, 50);
	}

	HealthBar::~HealthBar()
	{
		UnloadRenderTexture(healthBarTexture);
	}
}
