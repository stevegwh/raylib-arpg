#pragma once

#include "EditorGui.hpp"
#include "cereal/cereal.hpp"
#include "raylib.h"
#include "raymath.h"

#include <cstddef>
#include <string>
#include <vector>

namespace sage::editor
{
    struct PlaceableAsset
    {
        std::string displayName;
        std::string modelKey;
        std::string labelStem;
        float modelDefaultHeightOffset = 0.0f;
        float modelDefaultRotationY = 0.0f;
        float modelDefaultScale = 1.0f;
        Matrix appliedModelDefaultTransform = MatrixIdentity();
    };

    class EditorAssetCatalog
    {
        struct SerializedAssetDefaults
        {
            std::string displayName;
            std::string modelKey;
            float modelDefaultHeightOffset = 0.0f;
            float modelDefaultRotationY = 0.0f;
            float modelDefaultScale = 1.0f;
            Matrix appliedModelDefaultTransform = MatrixIdentity();

            template <class Archive>
            void serialize(Archive& archive)
            {
                archive(
                    CEREAL_NVP(displayName),
                    CEREAL_NVP(modelKey),
                    CEREAL_NVP(modelDefaultHeightOffset),
                    CEREAL_NVP(modelDefaultRotationY),
                    CEREAL_NVP(modelDefaultScale),
                    CEREAL_NVP(appliedModelDefaultTransform));
            }
        };

        std::vector<PlaceableAsset> assets;
        std::size_t selectedIndex = 0;

        [[nodiscard]] SerializedAssetDefaults serializeAssetDefaults(const PlaceableAsset& placeable) const;
        [[nodiscard]] std::string assetDefaultsPath(const PlaceableAsset& placeable) const;
        [[nodiscard]] PlaceableAsset& SelectedMutable();
        void loadAssetDefaults(PlaceableAsset& placeable) const;
        void saveAssetDefaults(const PlaceableAsset& placeable) const;

      public:
        explicit EditorAssetCatalog(std::vector<PlaceableAsset> placeables);

        void LoadDefaults();
        void Select(std::size_t index);
        void AdjustSelectedDefaultHeight(float amount);
        void AdjustSelectedDefaultRotation(float amount);
        void AdjustSelectedDefaultScale(float amount);
        void ApplySelectedDefaults();
        void ResetSelectedDefaults();

        [[nodiscard]] const PlaceableAsset& Selected() const;
        [[nodiscard]] std::size_t SelectedIndex() const;
        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] Matrix DefaultTransform(const PlaceableAsset& placeable) const;
        [[nodiscard]] Matrix SelectedDefaultTransform() const;
        [[nodiscard]] std::vector<EditorGui::AssetEntry> AssetEntries() const;
    };
} // namespace sage::editor
