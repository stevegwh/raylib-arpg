//
// Created by Steve Wheeler on 21/03/2024.
//

#include "Serializer.hpp"

#include "abilities/AbilityData.hpp"
#include "components/Collideable.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "components/Spawner.hpp"
#include "Light.hpp"
#include "ViewSerializer.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include "raylib-cereal.hpp"
#include "sage-cereal.hpp"

#include <cereal/archives/json.hpp>
#include <fstream>

namespace sage
{

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

        void SaveMap(entt::registry& source, const char* path)
        {
            std::cout << "START: Saving map data to file." << std::endl;
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

                ViewSerializer<Spawner> spawnerLoader(&source);
                output(spawnerLoader);

                ViewSerializer<Light> lightLoader(&source);
                output(lightLoader);

                output(ResourceManager::GetInstance());

                const auto view = source.view<sgTransform, Renderable, Collideable>();
                for (const auto& ent : view)
                {
                    const auto& rend = view.get<Renderable>(ent);
                    const auto& trans = view.get<sgTransform>(ent);
                    const auto& col = view.get<Collideable>(ent);

                    entity entity{};
                    entity.id = entt::entt_traits<entt::entity>::to_entity(ent);
                    output(entity, trans, col, rend);
                }
            }
            storage.close();
            std::cout << "FINISH: Saving map data to file." << std::endl;
        }

        void LoadMap(entt::registry* destination, const char* path)
        {
            assert(destination != nullptr);

            std::cout << "START: Loading resource data from file." << std::endl;

            using namespace entt::literals;
            std::ifstream storage(path, std::ios::binary);
            if (!storage.is_open())
            {
                std::cerr << "ERROR: Unable to open file for reading." << std::endl;
                exit(1);
            }

            {
                cereal::BinaryInputArchive input(storage);

                ViewSerializer<Spawner> spawnerLoader(destination);
                input(spawnerLoader);

                ViewSerializer<Light> lightLoader(destination);
                input(lightLoader);

                input(ResourceManager::GetInstance());

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
                        std::cerr << "ERROR: Serialization error: " << e.what() << std::endl;
                        break;
                    }
                }
            }

            storage.close();
            std::cout << "FINISH: Loading resource data from file." << std::endl;
        }

        void LoadAssetBinFIle(entt::registry* destination, const char* path)
        {
            assert(destination != nullptr);

            std::cout << "START: Loading resource data from file." << std::endl;

            using namespace entt::literals;
            std::ifstream storage(path, std::ios::binary);
            if (!storage.is_open())
            {
                std::cerr << "ERROR: Unable to open file for reading." << std::endl;
                exit(1);
            }

            {
                cereal::BinaryInputArchive input(storage);

                input(ResourceManager::GetInstance());

                // Not necessary for asset bin?
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
                        std::cerr << "ERROR: Serialization error: " << e.what() << std::endl;
                        break;
                    }
                }
            }

            storage.close();
            std::cout << "FINISH: Loading resource data from file." << std::endl;
        }

        void LoadAbilityData(AbilityData& abilityData, const char* path)
        {
            std::cout << "START: Loading data from file." << std::endl;
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
            std::cout << "FINISH: Loading data from file." << std::endl;
        };
    } // namespace serializer
} // namespace sage
