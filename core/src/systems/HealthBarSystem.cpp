//
// Created by steve on 20/05/2024.
//

#include "HealthBarSystem.hpp"

#include "Camera.hpp"
#include "components/Collideable.hpp"
#include "components/CombatableActor.hpp"
#include "components/HealthBar.hpp"

#include "raylib.h"

namespace sage
{
    void HealthBarSystem::Draw2D()
    {
    }

    void HealthBarSystem::updateHealthBarTextures() const
    {
        const auto& view = registry->view<HealthBar>();
        for (const auto& entity : view)
        {
            auto& hb = registry->get<HealthBar>(entity);
            const auto& c = registry->get<CombatableActor>(entity);
            const float decayRate = 2.0f; // Adjust this value to control the speed of decay
            hb.damageTaken *= exp(-decayRate * GetFrameTime());

            if (hb.damageTaken < 0.1f) hb.damageTaken = 0; // Reset to zero when it's very small

            float width = 200;
            float height = 20;
            BeginTextureMode(hb.healthBarTexture);
            ClearBackground(BLANK);

            DrawRectangle(0, 0, width, height, hb.healthBarBgColor);
            float healthPercentage = static_cast<float>(c.data.hp) / 100.0f;
            float damageTakenPercentage = static_cast<float>(hb.damageTaken) / 100.0f;
            int fillWidth = static_cast<int>(healthPercentage * width);
            DrawRectangle(0, 0, fillWidth, height, hb.healthBarColor);
            DrawRectangle(fillWidth, 0, static_cast<int>(damageTakenPercentage * 200), 20, WHITE);
            DrawRectangleLines(0, 0, width, height, hb.healthBarBorderColor);

            // Vector2 textSize = MeasureTextEx(GetFontDefault(), TextFormat("HP: %03i", c.data.hp), 20, 1);
            // DrawTextEx(
            //     GetFontDefault(),
            //     TextFormat("HP: %03i", c.data.hp),
            //     {10, hb.healthBarTexture.texture.height - 30 - textSize.y},
            //     20,
            //     1,
            //     GREEN);

            EndTextureMode();
        }
    }

    void HealthBarSystem::Draw3D()
    {
        const auto& view = registry->view<HealthBar, Collideable>();
        view.each([this](const auto& c, const auto& col) {
            const Vector3& min = col.worldBoundingBox.min;
            const Vector3& max = col.worldBoundingBox.max;

            Vector3 modelCenter = {min.x + (max.x - min.x) / 2, max.y, min.z + (max.z - min.z) / 2};

            Vector3 billboardPos = modelCenter;
            billboardPos.y += 1.0f;
            Rectangle sourceRec = {
                0.0f,
                0.0f,
                static_cast<float>(c.healthBarTexture.texture.width),
                static_cast<float>(-c.healthBarTexture.texture.height)};
            DrawBillboardRec(
                *camera->getRaylibCam(), c.healthBarTexture.texture, sourceRec, billboardPos, {3.0f, 1.0f}, WHITE);
        });
    }

    void HealthBarSystem::Update()
    {
        updateHealthBarTextures();
    }

    HealthBarSystem::HealthBarSystem(entt::registry* _registry, Camera* _camera)
        : registry(_registry), camera(_camera)
    {
    }
} // namespace sage
