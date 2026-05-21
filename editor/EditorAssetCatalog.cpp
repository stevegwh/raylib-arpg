#include "EditorAssetCatalog.hpp"

#include "engine/ResourceManager.hpp"
#include "engine/raylib-cereal.hpp"

#include "cereal/archives/json.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
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

        std::string AssetStemFromKey(const std::string& key)
        {
            constexpr std::array<std::string_view, 5> prefixes{"mdl_", "vfx_", "env_", "prop_", "item_"};
            for (const auto prefix : prefixes)
            {
                if (key.starts_with(prefix))
                {
                    return key.substr(prefix.size());
                }
            }
            return key;
        }

        std::string DisplayNameFromModelKey(const std::string& key)
        {
            auto stem = AssetStemFromKey(key);
            std::string result;
            result.reserve(stem.size());

            bool wordStart = true;
            for (const unsigned char ch : stem)
            {
                if (ch == '_' || ch == '-')
                {
                    result.push_back(' ');
                    wordStart = true;
                    continue;
                }

                result.push_back(static_cast<char>(wordStart ? std::toupper(ch) : std::tolower(ch)));
                wordStart = false;
            }

            return result.empty() ? key : result;
        }

        std::string LabelStemFromModelKey(const std::string& key)
        {
            auto stem = AssetStemFromKey(key);
            std::string result;
            result.reserve(stem.size());

            bool lastWasSeparator = false;
            for (const unsigned char ch : stem)
            {
                if (std::isalnum(ch))
                {
                    result.push_back(static_cast<char>(std::toupper(ch)));
                    lastWasSeparator = false;
                }
                else if (!lastWasSeparator)
                {
                    result.push_back('_');
                    lastWasSeparator = true;
                }
            }

            while (!result.empty() && result.back() == '_')
            {
                result.pop_back();
            }

            return result.empty() ? "ASSET" : result;
        }

        bool MatrixAlmostEqual(const Matrix& lhs, const Matrix& rhs)
        {
            constexpr float epsilon = 0.0001f;
            return std::fabs(lhs.m0 - rhs.m0) < epsilon && std::fabs(lhs.m1 - rhs.m1) < epsilon &&
                   std::fabs(lhs.m2 - rhs.m2) < epsilon && std::fabs(lhs.m3 - rhs.m3) < epsilon &&
                   std::fabs(lhs.m4 - rhs.m4) < epsilon && std::fabs(lhs.m5 - rhs.m5) < epsilon &&
                   std::fabs(lhs.m6 - rhs.m6) < epsilon && std::fabs(lhs.m7 - rhs.m7) < epsilon &&
                   std::fabs(lhs.m8 - rhs.m8) < epsilon && std::fabs(lhs.m9 - rhs.m9) < epsilon &&
                   std::fabs(lhs.m10 - rhs.m10) < epsilon && std::fabs(lhs.m11 - rhs.m11) < epsilon &&
                   std::fabs(lhs.m12 - rhs.m12) < epsilon && std::fabs(lhs.m13 - rhs.m13) < epsilon &&
                   std::fabs(lhs.m14 - rhs.m14) < epsilon && std::fabs(lhs.m15 - rhs.m15) < epsilon;
        }

        bool IsIdentityMatrix(const Matrix& matrix)
        {
            return MatrixAlmostEqual(matrix, MatrixIdentity());
        }

        Matrix ModelSpaceDefaultTransform(const std::string& modelKey)
        {
            auto model = ResourceManager::GetInstance().GetModelView(modelKey);
            const auto bounds = model.CalcLocalBoundingBox();
            const Vector3 center = Vector3Scale(Vector3Add(bounds.min, bounds.max), 0.5f);
            return MatrixTranslate(-center.x, -bounds.min.y, -center.z);
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

    EditorAssetCatalog EditorAssetCatalog::FromLoadedModels(const bool includeGenerated)
    {
        auto modelKeys = ResourceManager::GetInstance().GetModelKeys(includeGenerated);
        std::vector<PlaceableAsset> placeables;
        placeables.reserve(modelKeys.size());

        for (const auto& key : modelKeys)
        {
            const auto modelSpaceDefaultTransform = ModelSpaceDefaultTransform(key);
            placeables.push_back(PlaceableAsset{
                .displayName = DisplayNameFromModelKey(key),
                .modelKey = key,
                .labelStem = LabelStemFromModelKey(key),
                .appliedModelDefaultTransform = modelSpaceDefaultTransform,
                .modelSpaceDefaultTransform = modelSpaceDefaultTransform,
            });
        }

        return EditorAssetCatalog{std::move(placeables)};
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
        placeable.appliedModelDefaultTransform = placeable.modelSpaceDefaultTransform;
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
            if (!IsIdentityMatrix(placeable.modelSpaceDefaultTransform))
            {
                saveAssetDefaults(placeable);
            }
            return;
        }

        try
        {
            SerializedAssetDefaults defaults{};
            {
                std::ifstream inputFile(path);
                cereal::JSONInputArchive input(inputFile);
                input(cereal::make_nvp("assetDefaults", defaults));
            }

            if (!defaults.modelKey.empty() && defaults.modelKey != placeable.modelKey)
            {
                std::cerr << "WARN: Asset defaults model key mismatch in " << path << std::endl;
                return;
            }

            placeable.displayName = defaults.displayName.empty() ? placeable.displayName : defaults.displayName;
            placeable.modelDefaultHeightOffset = defaults.modelDefaultHeightOffset;
            placeable.modelDefaultRotationY = defaults.modelDefaultRotationY;
            placeable.modelDefaultScale = defaults.modelDefaultScale;
            if (IsIdentityMatrix(defaults.appliedModelDefaultTransform) &&
                !IsIdentityMatrix(placeable.modelSpaceDefaultTransform))
            {
                placeable.appliedModelDefaultTransform = placeable.modelSpaceDefaultTransform;
                saveAssetDefaults(placeable);
            }
            else
            {
                placeable.appliedModelDefaultTransform = defaults.appliedModelDefaultTransform;
            }
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
