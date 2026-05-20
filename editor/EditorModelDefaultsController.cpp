#include "EditorModelDefaultsController.hpp"

#include <format>
#include <utility>

namespace sage::editor
{
    namespace
    {
        constexpr float PLACEMENT_HEIGHT_STEP = 0.25f;
        constexpr float PLACEMENT_ROTATION_STEP = 15.0f;
        constexpr float PLACEMENT_SCALE_STEP = 0.1f;
    } // namespace

    EditorModelDefaultsController::EditorModelDefaultsController(
        EditorAssetCatalog& _assets,
        std::function<bool()> _isActive,
        std::function<void()> _onChanged)
        : assets(_assets), isActive(std::move(_isActive)), onChanged(std::move(_onChanged))
    {
    }

    void EditorModelDefaultsController::AdjustHeight(const float amount)
    {
        if (isActive && !isActive()) return;
        assets.AdjustSelectedDefaultHeight(amount);
        notifyChanged();
    }

    void EditorModelDefaultsController::AdjustRotation(const float amount)
    {
        if (isActive && !isActive()) return;
        assets.AdjustSelectedDefaultRotation(amount);
        notifyChanged();
    }

    void EditorModelDefaultsController::AdjustScale(const float amount)
    {
        if (isActive && !isActive()) return;
        assets.AdjustSelectedDefaultScale(amount);
        notifyChanged();
    }

    void EditorModelDefaultsController::Apply()
    {
        if (isActive && !isActive()) return;
        assets.ApplySelectedDefaults();
        notifyChanged();
    }

    void EditorModelDefaultsController::Reset()
    {
        if (isActive && !isActive()) return;
        assets.ResetSelectedDefaults();
        notifyChanged();
    }

    ModelDefaultsStatus EditorModelDefaultsController::Status(const std::string& inactiveAssetName) const
    {
        if (isActive && !isActive())
        {
            return {
                .assetName = inactiveAssetName,
                .height = "0.00",
                .rotation = "0",
                .scale = "1.00",
            };
        }

        const auto& placeable = assets.Selected();
        return {
            .assetName = placeable.displayName,
            .height = std::format("{:.2f}", placeable.modelDefaultHeightOffset),
            .rotation = std::format("{:.0f}", placeable.modelDefaultRotationY),
            .scale = std::format("{:.2f}", placeable.modelDefaultScale),
        };
    }

    EditorGui::ModelDefaultCallbacks EditorModelDefaultsController::Callbacks()
    {
        return {
            .heightDown = [this]() { AdjustHeight(-PLACEMENT_HEIGHT_STEP); },
            .heightUp = [this]() { AdjustHeight(PLACEMENT_HEIGHT_STEP); },
            .rotationDown = [this]() { AdjustRotation(-PLACEMENT_ROTATION_STEP); },
            .rotationUp = [this]() { AdjustRotation(PLACEMENT_ROTATION_STEP); },
            .scaleDown = [this]() { AdjustScale(-PLACEMENT_SCALE_STEP); },
            .scaleUp = [this]() { AdjustScale(PLACEMENT_SCALE_STEP); },
            .apply = [this]() { Apply(); },
            .reset = [this]() { Reset(); },
        };
    }

    void EditorModelDefaultsController::notifyChanged() const
    {
        if (onChanged) onChanged();
    }
} // namespace sage::editor
