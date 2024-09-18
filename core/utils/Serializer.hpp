//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "abilities/AbilityData.hpp"
#include "components/NavigationGridSquare.hpp"
#include "entt/entity/registry.hpp"
#include "KeyMapping.hpp"
#include "Settings.hpp"

namespace sage::serializer
{
    void SaveMap(entt::registry& source, const char* path);
    void LoadMap(entt::registry* destination, const char* path);
    void SaveCurrentResourceData(entt::registry& source, const char* path);
    void LoadBinFile(entt::registry* destination, const char* path);
    void SerializeKeyMapping(KeyMapping& keymapping, const char* path);
    void DeserializeKeyMapping(KeyMapping& keymapping, const char* path);
    void SerializeSettings(Settings& settings, const char* path);
    void DeserializeSettings(Settings& settings, const char* path);
    void SaveAbilityData(const AbilityData& abilityData, const char* path);
    void LoadAbilityData(AbilityData& abilityData, const char* path);
} // namespace sage::serializer
