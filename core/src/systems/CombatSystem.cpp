//
// Created by steve on 20/05/2024.
//

#include "CombatSystem.hpp"
#include "components/Collideable.hpp"

#include "raylib.h"


namespace sage
{
void CombatSystem::Draw2D()
{
}

void CombatSystem::updateHealthBarTexture()
{
    const auto& view = registry->view<Combat>();
    view.each([this](const auto& c)
              {
                  // Create a texture or render target for the health bar and text
                  BeginTextureMode(healthBarTexture);
                  ClearBackground(BLANK);

                  // Draw the health bar on the texture
                  DrawRectangle(0, 0, 200, 20, healthBarBgColor);
                  float healthPercentage = (float)c.hp / 100.0f;
                  int fillWidth = (int)(healthPercentage * 200);
                  DrawRectangle(0, 0, fillWidth, 20, healthBarColor);
                  DrawRectangleLines(0, 0, 200, 20, healthBarBorderColor);

                  // Draw the text upside-down on the texture
                  Vector2 textSize = MeasureTextEx(GetFontDefault(), TextFormat("HP: %03i", c.hp), 20, 1);
                  DrawTextEx(GetFontDefault(), TextFormat("HP: %03i", c.hp), (Vector2){ 10, healthBarTexture.texture.height - 30 - textSize.y }, 20, 1, GREEN);

                  EndTextureMode();
              });
}

void CombatSystem::Draw3D()
{
    const auto& view = registry->view<Combat, Collideable>();
    view.each([this](const auto& c, const auto& col)
              {
                  const Vector3& min = col.worldBoundingBox.min;
                  const Vector3& max = col.worldBoundingBox.max;

                  Vector3 modelCenter = {
                      min.x + (max.x - min.x) / 2,
                      max.y,
                      min.z + (max.z - min.z) / 2
                  };

                  // Create a billboard quad above the model
                  Vector3 billboardPos = modelCenter;
                  billboardPos.y += 1.0f; // Adjust the height above the model

                  // Define the source rectangle for the texture
                  Rectangle sourceRec = { 0.0f, 0.0f, (float)healthBarTexture.texture.width, (float)-healthBarTexture.texture.height };

                  // Draw the billboard quad with the health bar texture
                  DrawBillboardRec(*camera->getRaylibCam(), healthBarTexture.texture, sourceRec, billboardPos, {1.0f, 1.0f}, WHITE);
              });
}

void CombatSystem::Update()
{
    updateHealthBarTexture();
}

CombatSystem::~CombatSystem()
{
    UnloadRenderTexture(healthBarTexture);
}

CombatSystem::CombatSystem(entt::registry* _registry, sage::Camera* _camera) : 
BaseSystem<Combat>(_registry), camera(_camera)
{
    healthBarTexture = LoadRenderTexture(200, 50);
}
} // sage