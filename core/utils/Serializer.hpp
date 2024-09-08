//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "../src/KeyMapping.hpp"
#include "abilities/AbilityData.hpp"
#include "components/NavigationGridSquare.hpp"
#include "entt/entity/registry.hpp"
#include "Settings.hpp"

namespace sage::serializer
{
    void SaveMap(const entt::registry& registry, ImageSafe& heightMap, ImageSafe& normalMap);
    void LoadMap(entt::registry* destination, ImageSafe& heightMap, ImageSafe& normalMap);
    void SerializeKeyMapping(KeyMapping& keymapping, const char* path);
    void DeserializeKeyMapping(KeyMapping& keymapping, const char* path);
    void SerializeSettings(Settings& settings, const char* path);
    void DeserializeSettings(Settings& settings, const char* path);
    void GenerateNormalMap(
        entt::registry* registry,
        const std::vector<std::vector<NavigationGridSquare*>>& gridSquares,
        ImageSafe& image);
    float GetMaxHeight(entt::registry* registry, float slices);
    void GenerateHeightMap(
        entt::registry* registry,
        const std::vector<std::vector<NavigationGridSquare*>>& gridSquares,
        ImageSafe& image);
    void SaveAbilityData(const AbilityData& abilityData, const char* path);
    void LoadAbilityData(AbilityData& abilityData, const char* path);
} // namespace sage::serializer
