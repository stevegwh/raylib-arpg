//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "entt/entity/registry.hpp"
#include "KeyMapping.hpp"
#include "Settings.hpp"

namespace sage::serializer
{
void Save(const entt::registry& registry);
void Load(entt::registry* registry);
void SerializeKeyMapping(KeyMapping& keymapping, const char* path);
void DeserializeKeyMapping(KeyMapping& keymapping, const char* path);
void SerializeSettings(Settings& settings, const char* path);
void DeserializeSettings(Settings& settings, const char* path);
}



