#include "EditorMapLoader.hpp"

#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/Spawner.hpp"
#include "engine/Light.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/Serializer.hpp"

#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace sage::editor
{
    namespace
    {
        // Matches the wire layout of lq::ItemComponent::load (4 strings, vector<string>, 2 strings).
        // Used to advance the binary stream past item payloads without depending on the game module.
        struct ItemComponentSkip
        {
            std::string name;
            std::string localizedName;
            std::string description;
            std::string rarity;
            std::vector<std::string> flagNames;
            std::string icon;
            std::string model;

            template <class Archive>
            void serialize(Archive& archive)
            {
                archive(name, localizedName, description, rarity, flagNames, icon, model);
            }
        };
    } // namespace

    void LoadMap(entt::registry* destination, const char* path)
    {
        assert(destination != nullptr);
        std::cout << "START: Loading map data from file (editor)." << std::endl;

        std::unordered_map<std::uint32_t, entt::entity> idMap;

        sage::serializer::ReadCompressedBinary(
            path, sage::serializer::kMapBinMagic, [&](cereal::BinaryInputArchive& input, std::istream& stream) {
                std::vector<Spawner> spawners;
                input(spawners);
                for (const auto& spawner : spawners)
                {
                    const auto entity = destination->create();
                    destination->emplace<Spawner>(entity, spawner);
                }

                std::vector<Light> lights;
                input(lights);
                for (const auto& light : lights)
                {
                    const auto entity = destination->create();
                    destination->emplace<Light>(entity, light);
                }

                input(ResourceManager::GetInstance());

                unsigned int itemCount = 0;
                input(itemCount);

                for (unsigned int i = 0; i < itemCount; ++i)
                {
                    const auto entity = destination->create();
                    sage::serializer::entity entityId{};
                    auto& transform = destination->emplace<sgTransform>(entity);
                    auto& collideable = destination->emplace<Collideable>(entity);
                    collideable.isStatic = true;
                    auto& renderable = destination->emplace<Renderable>(entity);
                    ItemComponentSkip skip{};

                    try
                    {
                        input(entityId, transform, collideable, renderable, skip);
                    }
                    catch (const cereal::Exception& e)
                    {
                        std::cerr << "ERROR: Serialization error (item): " << e.what() << std::endl;
                        return;
                    }
                    idMap[entityId.id] = entity;
                }

                while (stream.peek() != EOF)
                {
                    const auto entity = destination->create();
                    sage::serializer::entity entityId{};
                    auto& transform = destination->emplace<sgTransform>(entity);
                    auto& collideable = destination->emplace<Collideable>(entity);
                    collideable.isStatic = true;
                    auto& renderable = destination->emplace<Renderable>(entity);

                    try
                    {
                        input(entityId, transform, collideable, renderable);
                    }
                    catch (const cereal::Exception& e)
                    {
                        std::cerr << "ERROR: Serialization error (entity): " << e.what() << std::endl;
                        return;
                    }
                    idMap[entityId.id] = entity;
                }
            });

        for (auto [e, t] : destination->view<sgTransform>().each())
        {
            t.ResolveSerializedParent(idMap);
        }

        std::cout << "FINISH: Loading map data from file (editor)." << std::endl;
    }
} // namespace sage::editor
