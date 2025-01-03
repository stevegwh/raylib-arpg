//
// Created by Steve Wheeler on 03/01/2025.
//

#include "ContextualDialogSystem.hpp"

#include "Camera.hpp"
#include "components/ContextualDialogComponent.hpp"
#include "GameData.hpp"

#include "components/Collideable.hpp"
#include "raylib.h"
#include "Settings.hpp"
#include "slib.hpp"

namespace sage
{

    void ContextualDialogSystem::Draw2D() const
    {
        for (const auto view = registry->view<ContextualDialogComponent>(); const auto& entity : view)
        {
            const auto& col = registry->get<Collideable>(entity);
            auto viewport = gameData->settings->GetViewPort();
            auto center =
                Vector3MultiplyByValue(Vector3Add(col.worldBoundingBox.max, col.worldBoundingBox.min), 0.5f);
            auto pos = Vector3{center.x, col.worldBoundingBox.max.y + 1.0f, center.z};
            auto screenPos = GetWorldToScreenEx(pos, *gameData->camera->getRaylibCam(), viewport.x, viewport.y);
            auto& contextualDiag = registry->get<ContextualDialogComponent>(entity);

            const auto text = contextualDiag.GetText();
            DrawText(
                text.c_str(),
                static_cast<int>(screenPos.x) - MeasureText(text.c_str(), 16) / 2,
                static_cast<int>(screenPos.y),
                20,
                WHITE);
        }
    }

    void ContextualDialogSystem::Update() const
    {
    }

    ContextualDialogSystem::ContextualDialogSystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

} // namespace sage