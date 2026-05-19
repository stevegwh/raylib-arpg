#include "EditorAssetCatalog.hpp"

#include "engine/raylib-cereal.hpp"

#include "cereal/archives/json.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

namespace sage::editor
{
    namespace
    {
        constexpr float PLACEMENT_MIN_SCALE = 0.1f;
        const std::filesystem::path IMPORTED_ASSETS_DIRECTORY{"resources/Editor/ImportedAssets"};

        std::string SanitizeAssetFileStem(const std::string& input)
        {
            std::string result;
            result.reserve(input.size());

            for (const unsigned char ch : input)
            {
                if (std::isalnum(ch) || ch == '-' || ch == '_')
                {
                    result.push_back(static_cast<char>(ch));
                }
                else
                {
                    result.push_back('_');
                }
            }

            return result.empty() ? "asset" : result;
        }

        void WrapDegrees(float& degrees)
        {
            if (degrees >= 360.0f) degrees -= 360.0f;
            if (degrees < 0.0f) degrees += 360.0f;
        }
    } // namespace

    EditorAssetCatalog::EditorAssetCatalog(std::vector<PlaceableAsset> placeables)
        : assets(std::move(placeables))
    {
    }

    void EditorAssetCatalog::LoadDefaults()
    {
        std::filesystem::create_directories(IMPORTED_ASSETS_DIRECTORY);
        for (auto& placeable : assets)
        {
            loadAssetDefaults(placeable);
        }
    }

    void EditorAssetCatalog::Select(const std::size_t index)
    {
        if (index >= assets.size()) return;
        selectedIndex = index;
    }

    void EditorAssetCatalog::AdjustSelectedDefaultHeight(const float amount)
    {
        SelectedMutable().modelDefaultHeightOffset += amount;
    }

    void EditorAssetCatalog::AdjustSelectedDefaultRotation(const float amount)
    {
        auto& rotationY = SelectedMutable().modelDefaultRotationY;
        rotationY += amount;
        WrapDegrees(rotationY);
    }

    void EditorAssetCatalog::AdjustSelectedDefaultScale(const float amount)
    {
        auto& scale = SelectedMutable().modelDefaultScale;
        scale = std::max(PLACEMENT_MIN_SCALE, scale + amount);
    }

    void EditorAssetCatalog::ApplySelectedDefaults()
    {
        auto& placeable = SelectedMutable();
        placeable.appliedModelDefaultTransform = DefaultTransform(placeable);
        placeable.modelDefaultHeightOffset = 0.0f;
        placeable.modelDefaultRotationY = 0.0f;
        placeable.modelDefaultScale = 1.0f;
        saveAssetDefaults(placeable);
    }

    void EditorAssetCatalog::ResetSelectedDefaults()
    {
        auto& placeable = SelectedMutable();
        placeable.appliedModelDefaultTransform = MatrixIdentity();
        placeable.modelDefaultHeightOffset = 0.0f;
        placeable.modelDefaultRotationY = 0.0f;
        placeable.modelDefaultScale = 1.0f;
        saveAssetDefaults(placeable);
    }

    const PlaceableAsset& EditorAssetCatalog::Selected() const
    {
        return assets.at(selectedIndex);
    }

    std::size_t EditorAssetCatalog::SelectedIndex() const
    {
        return selectedIndex;
    }

    std::size_t EditorAssetCatalog::Size() const
    {
        return assets.size();
    }

    Matrix EditorAssetCatalog::DefaultTransform(const PlaceableAsset& placeable) const
    {
        const Matrix editableTransform = MatrixMultiply(
            MatrixMultiply(
                MatrixScale(placeable.modelDefaultScale, placeable.modelDefaultScale, placeable.modelDefaultScale),
                MatrixRotateY(placeable.modelDefaultRotationY * DEG2RAD)),
            MatrixTranslate(0.0f, placeable.modelDefaultHeightOffset, 0.0f));

        return MatrixMultiply(editableTransform, placeable.appliedModelDefaultTransform);
    }

    Matrix EditorAssetCatalog::SelectedDefaultTransform() const
    {
        return DefaultTransform(Selected());
    }

    std::vector<EditorGui::AssetEntry> EditorAssetCatalog::AssetEntries() const
    {
        std::vector<EditorGui::AssetEntry> entries;
        entries.reserve(assets.size());
        for (const auto& placeable : assets)
        {
            entries.push_back({placeable.displayName, placeable.modelKey});
        }
        return entries;
    }

    EditorAssetCatalog::SerializedAssetDefaults EditorAssetCatalog::serializeAssetDefaults(
        const PlaceableAsset& placeable) const
    {
        return {
            .displayName = placeable.displayName,
            .modelKey = placeable.modelKey,
            .modelDefaultHeightOffset = placeable.modelDefaultHeightOffset,
            .modelDefaultRotationY = placeable.modelDefaultRotationY,
            .modelDefaultScale = placeable.modelDefaultScale,
            .appliedModelDefaultTransform = placeable.appliedModelDefaultTransform,
        };
    }

    PlaceableAsset& EditorAssetCatalog::SelectedMutable()
    {
        return assets.at(selectedIndex);
    }

    std::string EditorAssetCatalog::assetDefaultsPath(const PlaceableAsset& placeable) const
    {
        return (IMPORTED_ASSETS_DIRECTORY / (SanitizeAssetFileStem(placeable.modelKey) + ".json")).string();
    }

    void EditorAssetCatalog::loadAssetDefaults(PlaceableAsset& placeable) const
    {
        const auto path = assetDefaultsPath(placeable);
        if (!std::filesystem::exists(path))
        {
            saveAssetDefaults(placeable);
            return;
        }

        try
        {
            std::ifstream inputFile(path);
            cereal::JSONInputArchive input(inputFile);
            SerializedAssetDefaults defaults{};
            input(cereal::make_nvp("assetDefaults", defaults));

            if (!defaults.modelKey.empty() && defaults.modelKey != placeable.modelKey)
            {
                std::cerr << "WARN: Asset defaults model key mismatch in " << path << std::endl;
                return;
            }

            placeable.displayName = defaults.displayName.empty() ? placeable.displayName : defaults.displayName;
            placeable.modelDefaultHeightOffset = defaults.modelDefaultHeightOffset;
            placeable.modelDefaultRotationY = defaults.modelDefaultRotationY;
            placeable.modelDefaultScale = defaults.modelDefaultScale;
            placeable.appliedModelDefaultTransform = defaults.appliedModelDefaultTransform;
        }
        catch (const cereal::Exception& e)
        {
            std::cerr << "WARN: Failed to load asset defaults from " << path << ": " << e.what() << std::endl;
        }
    }

    void EditorAssetCatalog::saveAssetDefaults(const PlaceableAsset& placeable) const
    {
        std::filesystem::create_directories(IMPORTED_ASSETS_DIRECTORY);

        const auto path = assetDefaultsPath(placeable);
        std::ofstream outputFile(path);
        if (!outputFile.is_open())
        {
            std::cerr << "ERROR: Unable to save asset defaults to " << path << std::endl;
            return;
        }

        cereal::JSONOutputArchive output(outputFile);
        auto defaults = serializeAssetDefaults(placeable);
        output(cereal::make_nvp("assetDefaults", defaults));
    }
} // namespace sage::editor
