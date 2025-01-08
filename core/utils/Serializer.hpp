//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "abilities/AbilityData.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include <cereal/archives/json.hpp>

#include "entt/entt.hpp"
#include <fstream>

namespace sage::serializer
{
    void SaveMap(entt::registry& source, const char* path);
    void LoadMap(entt::registry* destination, const char* path);
    void LoadAssetBinFile(entt::registry* destination, const char* path);
    void LoadAbilityData(AbilityData& abilityData, const char* path);

    template <typename T>
    void SaveClassXML(const char* path, const T& toSave)
    {
        std::cout << "START: Saving class data to XML file." << std::endl;
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
            cereal::XMLOutputArchive output{storage};
            output(toSave);
        }

        storage.close();
        std::cout << "FINISH: Saving class data to XML file." << std::endl;
    }

    template <typename T>
    void DeserializeXMLFile(const char* path, T& target)
    {
        std::cout << "START: Loading data from file." << std::endl;
        using namespace entt::literals;

        std::ifstream storage(path);
        if (storage.is_open())
        {
            cereal::XMLInputArchive input{storage};
            input(target);
            storage.close();
        }
        else
        {
            // File doesn't exist, create a new file with the default key mapping
            std::cout << "INFO: File not found. Creating a new file with defaults." << std::endl;
            SaveClassXML<T>(path, target);
        }
        std::cout << "FINISH: Loading data from file." << std::endl;
    }

    template <typename T>
    void SaveClassJson(const char* path, const T& toSave)
    {
        std::cout << "START: Saving class data to json file." << std::endl;
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
            // output.setNextName(name);
            output(toSave);
        }

        storage.close();
        std::cout << "FINISH: Saving class data to json file." << std::endl;
    }

    template <typename T>
    void DeserializeJsonFile(const char* path, T& target)
    {
        std::cout << "START: Loading data from file." << std::endl;
        using namespace entt::literals;

        std::ifstream storage(path);
        if (storage.is_open())
        {
            cereal::JSONInputArchive input{storage};
            input(target);
            storage.close();
        }
        else
        {
            // File doesn't exist, create a new file with the default key mapping
            std::cout << "INFO: File not found. Creating a new file with defaults." << std::endl;
            SaveClassJson<T>(path, target);
        }
        std::cout << "FINISH: Loading data from file." << std::endl;
    }

    template <typename T>
    void SaveClassBinary(const char* path, const T& toSave)
    {
        std::cout << "START: Saving class data to binary file." << std::endl;
        using namespace entt::literals;
        // std::stringstream storage;

        std::ofstream storage(path, std::ios::binary);
        if (!storage.is_open())
        {
            std::cerr << "ERROR: Unable to open file for writing." << std::endl;
            exit(1);
        }

        {
            // output finishes flushing its contents when it goes out of scope
            cereal::BinaryOutputArchive output{storage};
            output(toSave);
        }

        storage.close();
        std::cout << "FINISH: Saving class data to binary file." << std::endl;
    }
} // namespace sage::serializer
