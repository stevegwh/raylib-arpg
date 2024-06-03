//
// Created by steve on 20/05/2024.
//

#include "HealthBarSystem.hpp"
#include "components/Collideable.hpp"

#include "raylib.h"


namespace sage
{
void HealthBarSystem::Draw2D()
{
}

void HealthBarSystem::updateHealthBarTexture()
{
    const auto& view = registry->view<HealthBar>();
    view.each([this](const auto& c)
              {
                  BeginTextureMode(healthBarTexture);
                  ClearBackground(BLANK);
                  
                  DrawRectangle(0, 0, 200, 20, healthBarBgColor);
                  float healthPercentage = (float)c.hp / 100.0f;
                  int fillWidth = (int)(healthPercentage * 200);
                  DrawRectangle(0, 0, fillWidth, 20, healthBarColor);
                  DrawRectangleLines(0, 0, 200, 20, healthBarBorderColor);
                  
                  Vector2 textSize = MeasureTextEx(GetFontDefault(), TextFormat("HP: %03i", c.hp), 20, 1);
                  DrawTextEx(GetFontDefault(), TextFormat("HP: %03i", c.hp), (Vector2){ 10, healthBarTexture.texture.height - 30 - textSize.y }, 20, 1, GREEN);

                  EndTextureMode();
              });
}

void HealthBarSystem::Draw3D()
{
    const auto& view = registry->view<HealthBar, Collideable>();
    view.each([this](const auto& c, const auto& col)
              {
                  const Vector3& min = col.worldBoundingBox.min;
                  const Vector3& max = col.worldBoundingBox.max;

                  Vector3 modelCenter = {
                      min.x + (max.x - min.x) / 2,
                      max.y,
                      min.z + (max.z - min.z) / 2
                  };
                  
                  Vector3 billboardPos = modelCenter;
                  billboardPos.y += 1.0f;
                  Rectangle sourceRec = { 0.0f, 0.0f, (float)healthBarTexture.texture.width, (float)-healthBarTexture.texture.height };
                  DrawBillboardRec(*camera->getRaylibCam(), healthBarTexture.texture, sourceRec, billboardPos, {1.0f, 1.0f}, WHITE);
              });
}

void HealthBarSystem::Update()
{
    updateHealthBarTexture();
}

HealthBarSystem::~HealthBarSystem()
{
    UnloadRenderTexture(healthBarTexture);
}

HealthBarSystem::HealthBarSystem(entt::registry* _registry, sage::Camera* _camera) :
    BaseSystem<HealthBar>(_registry), camera(_camera)
{
    healthBarTexture = LoadRenderTexture(200, 50);
}
} // sage