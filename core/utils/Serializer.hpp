//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "abilities/AbilityData.hpp"
#include "components/NavigationGridSquare.hpp"
#include "KeyMapping.hpp"
#include "Settings.hpp"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include <cereal/archives/json.hpp>

#include <entt/entt.hpp>
#include <fstream>

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

    template <typename T>
    void SaveClassJson(const char* path, const char* name, T toSave)
    {
        std::cout << "START: Save resource data to file." << std::endl;
        using namespace entt::literals;
        // std::stringstream storage;

        std::ofstream storage(path);
        if (!storage.is_open())
        {
            std::cerr << "ERROR: Unable to open file for writing." << std::endl;
            exit(1);
        }

        {
            // output finishes flushing its contents when it goes out of scope
            cereal::JSONOutputArchive output{storage};
            output.setNextName(name);
            output(toSave);
        }

        storage.close();
        std::cout << "FINISH: Save resource data to file." << std::endl;
    }
} // namespace sage::serializer
