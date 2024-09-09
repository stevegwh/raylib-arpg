//
// Created by Steve Wheeler on 21/03/2024.
//

#include "Serializer.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include "entt/entity/snapshot.hpp"
#include "raylib-cereal.hpp"
#include "raylib.h"
#include <cereal/archives/json.hpp>
#include <fstream>
#include <type_traits>
#include <vector>

#include "abilities/AbilityData.hpp"
#include "abilities/AbilityResourceManager.hpp"
#include "components/Collideable.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"

#include <algorithm>

namespace sage
{
    template <typename Archive>
    void save(Archive& archive, AbilityData::BaseData const& bd)
    {
        archive(
            cereal::make_nvp("cooldownDuration", bd.cooldownDuration),
            cereal::make_nvp("baseDamage", bd.baseDamage),
            cereal::make_nvp("range", bd.range),
            cereal::make_nvp("radius", bd.radius)
            // cereal::make_nvp("element", bd.element),
            // cereal::make_nvp("repeatable", bd.repeatable)
            // cereal::make_nvp("executeFuncName", bd.executeFuncName)
        );
    }

    template <typename Archive>
    void load(Archive& archive, AbilityData::BaseData& bd)
    {
        archive(
            cereal::make_nvp("cooldownDuration", bd.cooldownDuration),
            cereal::make_nvp("baseDamage", bd.baseDamage),
            cereal::make_nvp("range", bd.range),
            cereal::make_nvp("radius", bd.radius)
            // cereal::make_nvp("element", bd.element),
            // cereal::make_nvp("repeatable", bd.repeatable)
            // cereal::make_nvp("executeFuncName", bd.executeFuncName)
        );
    }

    template <typename Archive>
    void serialize(Archive& archive, AbilityData::VisualFXData& vfx)
    {
        archive(cereal::make_nvp("name", vfx.name));
    }

    template <typename Archive>
    void serialize(Archive& archive, AnimationParams& anim)
    {
        archive(
            cereal::make_nvp("animEnum", anim.animEnum),
            cereal::make_nvp("animSpeed", anim.animSpeed),
            cereal::make_nvp("oneShot", anim.oneShot),
            cereal::make_nvp("animationDelay", anim.animationDelay));
    }

    template <class Archive>
    void save(Archive& archive, const AbilityData& ad)
    {
        archive(
            cereal::make_nvp("base", ad.base),
            cereal::make_nvp("animationParams", ad.animationParams),
            cereal::make_nvp("vfx", ad.vfx));
    }

    template <class Archive>
    void load(Archive& archive, AbilityData& ad)
    {
        archive(
            cereal::make_nvp("base", ad.base),
            cereal::make_nvp("animationParams", ad.animationParams),
            cereal::make_nvp("vfx", ad.vfx));
    }

    namespace serializer
    {
        // Archiving definitions
        struct entity
        {
            unsigned int id;
        };

        template <typename Archive>
        void serialize(Archive& archive, entity& entity)
        {
            archive(entity.id);
        }

        // ----------------------------------------------

        void SaveMap(const entt::registry& source, ImageSafe& heightMap, ImageSafe& normalMap)
        {
            std::cout << "Save called" << std::endl;
            using namespace entt::literals;
            // std::stringstream storage;

            std::ofstream storage("resources/output.bin", std::ios::binary);
            if (!storage.is_open())
            {
                // Handle file opening error
                return;
            }

            {
                // output finishes flushing its contents when it goes out of scope
                cereal::BinaryOutputArchive output{storage};

                output(heightMap, normalMap);
                output(ResourceManager::GetInstance());

                const auto view = source.view<sgTransform, Renderable, Collideable>();
                for (const auto& ent : view)
                {
                    const auto& rend = view.get<Renderable>(ent);
                    // if (!rend.serializable) continue;
                    const auto& trans = view.get<sgTransform>(ent);

                    const auto& col = view.get<Collideable>(ent);
                    entity entity{};
                    entity.id = entt::entt_traits<entt::entity>::to_entity(ent);
                    output(entity, trans, col, rend);
                }
            }
            storage.close();
            std::cout << "Save finished" << std::endl;
        }

        void LoadMap(entt::registry* destination, ImageSafe& heightMap, ImageSafe& normalMap)
        {
            assert(destination != nullptr);

            std::cout << "Load called" << std::endl;
            using namespace entt::literals;
            std::ifstream storage("resources/output.bin", std::ios::binary);
            if (!storage.is_open())
            {
                std::cerr << "Error: Unable to open file for reading." << std::endl;
                return;
            }

            {
                cereal::BinaryInputArchive input(storage);

                input(heightMap, normalMap);
                input(ResourceManager::GetInstance());
                // Use height/normal map to initiate navigation grid

                while (storage.peek() != EOF)
                {
                    entity entityId{}; // ignore this
                    auto entt = destination->create();
                    auto& transform = destination->emplace<sgTransform>(entt, entt);
                    auto& collideable = destination->emplace<Collideable>(entt);
                    auto& renderable = destination->emplace<Renderable>(entt);

                    try
                    {
                        input(entityId, transform, collideable, renderable);
                    }
                    catch (const cereal::Exception& e)
                    {
                        std::cerr << "Error during deserialization: " << e.what() << std::endl;
                        break;
                    }
                }
            }

            storage.close();
            std::cout << "Load finished" << std::endl;
        }

        void SerializeKeyMapping(KeyMapping& keymapping, const char* path)
        {
            std::cout << "Save called" << std::endl;
            using namespace entt::literals;
            // std::stringstream storage;

            std::ofstream storage(path);
            if (!storage.is_open())
            {
                // Handle file opening error
                return;
            }

            {
                // output finishes flushing its contents when it goes out of scope
                cereal::XMLOutputArchive output{storage};
                output(keymapping);
            }
            storage.close();
            std::cout << "Save finished" << std::endl;
        }

        void DeserializeKeyMapping(KeyMapping& keymapping, const char* path)
        {
            std::cout << "Load called" << std::endl;
            using namespace entt::literals;

            std::ifstream storage(path);
            if (storage.is_open())
            {
                cereal::XMLInputArchive input{storage};
                input(keymapping);
                storage.close();
            }
            else
            {
                // File doesn't exist, create a new file with the default key mapping
                std::cout << "Key mapping file not found. Creating a new file with the "
                             "default key mapping."
                          << std::endl;
                SerializeKeyMapping(keymapping, path);
            }
            std::cout << "Load finished" << std::endl;
        }

        void SerializeSettings(Settings& settings, const char* path)
        {
            std::cout << "Save called" << std::endl;
            using namespace entt::literals;
            // std::stringstream storage;

            std::ofstream storage(path);
            if (!storage.is_open())
            {
                // Handle file opening error
                return;
            }

            {
                // output finishes flushing its contents when it goes out of scope
                cereal::XMLOutputArchive output{storage};
                output(settings);
            }
            storage.close();
            std::cout << "Save finished" << std::endl;
        }

        void DeserializeSettings(Settings& settings, const char* path)
        {
            std::cout << "Load called" << std::endl;
            using namespace entt::literals;

            std::ifstream storage(path);
            if (storage.is_open())
            {
                cereal::XMLInputArchive input{storage};
                input(settings);
                storage.close();
            }
            else
            {
                // File doesn't exist, create a new file with the default key mapping
                std::cout << "Settings file not found. Creating a new file with default settings." << std::endl;
                SerializeSettings(settings, path);
            }
            std::cout << "Load finished" << std::endl;
        }

        void SaveAbilityData(const AbilityData& abilityData, const char* path)
        {
            std::cout << "Save called" << std::endl;
            using namespace entt::literals;
            // std::stringstream storage;

            std::ofstream storage(path);
            if (!storage.is_open())
            {
                // Handle file opening error
                return;
            }

            {
                // output finishes flushing its contents when it goes out of scope
                cereal::JSONOutputArchive output{storage};
                output(abilityData);
            }
            storage.close();
            std::cout << "Save finished" << std::endl;
        };

        void LoadAbilityData(AbilityData& abilityData, const char* path)
        {
            std::cout << "Load ability called" << std::endl;
            using namespace entt::literals;

            std::ifstream storage(path);
            if (storage.is_open())
            {
                cereal::JSONInputArchive input{storage};
                input(abilityData);
                storage.close();
            }
            // else
            // {
            //     // File doesn't exist, create a new file with the default key mapping
            //     std::cout << "Key mapping file not found. Creating a new file with the "
            //                  "default key mapping."
            //               << std::endl;
            //     SaveAbilityData(abilityData, path);
            // }
            std::cout << "Load ability finished" << std::endl;
        };
    } // namespace serializer
} // namespace sage
