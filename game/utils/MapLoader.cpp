#include "MapLoader.hpp"

#include "engine/components/Collideable.hpp"
#include "engine/components/DoorBehaviorComponent.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/Spawner.hpp"
#include "engine/Light.hpp"
#include "engine/Serializer.hpp"
#include "engine/ViewSerializer.hpp"

#include "components/DialogComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "Systems.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "engine/raylib-cereal.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include <cereal/archives/json.hpp>
#include <fstream>

namespace lq::maploader
{
    void SaveMap(entt::registry& source, const char* path)
    {
        std::cout << "START: Saving map data to file." << std::endl;

        sage::serializer::WriteCompressedBinary(
            path, sage::serializer::kMapBinMagic, [&](cereal::BinaryOutputArchive& output) {
                sage::ViewSerializer<sage::Spawner> spawnerLoader(&source);
                output(spawnerLoader);

                sage::ViewSerializer<sage::Light> lightLoader(&source);
                output(lightLoader);

                output(sage::ResourceManager::GetInstance());

                // Note: ViewSerializer creates separate entities per component type, so it can't
                // reconstruct multi-component entities (Renderable+Collideable+sgTransform must share
                // one entity). Per-entity serialization is used instead.
                const auto itemView =
                    source.view<sage::Renderable, sage::sgTransform, sage::Collideable, ItemComponent>();
                unsigned int itemCount = 0;
                for (const auto& entity : itemView)
                    ++itemCount;
                output(itemCount);

                for (const auto& ent : itemView)
                {
                    const auto& rend = source.get<sage::Renderable>(ent);
                    const auto& trans = source.get<sage::sgTransform>(ent);
                    const auto& col = source.get<sage::Collideable>(ent);
                    const auto& item = source.get<ItemComponent>(ent);

                    sage::serializer::entity entity{};
                    entity.id = entt::entt_traits<entt::entity>::to_entity(ent);
                    output(entity, trans, col, rend, item);
                }

                const auto view = source.view<sage::sgTransform, sage::Renderable, sage::Collideable>(
                    entt::exclude<ItemComponent>);
                for (const auto& ent : view)
                {
                    const auto& rend = view.get<sage::Renderable>(ent);
                    const auto& trans = view.get<sage::sgTransform>(ent);
                    const auto& col = view.get<sage::Collideable>(ent);

                    sage::serializer::entity entity{};
                    entity.id = entt::entt_traits<entt::entity>::to_entity(ent);
                    output(entity, trans, col, rend);
                }
            });

        std::cout << "FINISH: Saving map data to file." << std::endl;
    }

    void LoadMap(entt::registry* destination, const char* path)
    {
        assert(destination != nullptr);
        std::cout << "START: Loading map data from file." << std::endl;

        sage::serializer::ReadCompressedBinary(
            path, sage::serializer::kMapBinMagic, [&](cereal::BinaryInputArchive& input, std::istream& stream) {
                sage::ViewSerializer<sage::Spawner> spawnerLoader(destination);
                input(spawnerLoader);

                sage::ViewSerializer<sage::Light> lightLoader(destination);
                input(lightLoader);

                input(sage::ResourceManager::GetInstance());

                unsigned int itemCount;
                input(itemCount);

                for (unsigned int i = 0; i < itemCount; ++i)
                {
                    auto entt = destination->create();
                    sage::serializer::entity entityId{}; // ignore this (old serialized entity)
                    auto& transform = destination->emplace<sage::sgTransform>(entt);
                    auto& collideable = destination->emplace<sage::Collideable>(entt);
                    auto& renderable = destination->emplace<sage::Renderable>(entt);
                    auto& item = destination->emplace<ItemComponent>(entt);

                    try
                    {
                        input(entityId, transform, collideable, renderable, item);
                        collideable.isStatic = true;
                    }
                    catch (const cereal::Exception& e)
                    {
                        std::cerr << "ERROR: Serialization error: " << e.what() << std::endl;
                        break;
                    }
                }

                while (stream.peek() != EOF)
                {
                    sage::serializer::entity entityId{}; // ignore this (old serialized entity)
                    auto entt = destination->create();
                    auto& transform = destination->emplace<sage::sgTransform>(entt);
                    auto& collideable = destination->emplace<sage::Collideable>(entt);
                    auto& renderable = destination->emplace<sage::Renderable>(entt);

                    try
                    {
                        input(entityId, transform, collideable, renderable);
                        collideable.isStatic = true;
                    }
                    catch (const cereal::Exception& e)
                    {
                        std::cerr << "ERROR: Serialization error: " << e.what() << std::endl;
                        break;
                    }

                    if (renderable.GetName().find("_DOOR_") != std::string::npos)
                    {
                        destination->emplace<sage::DoorBehaviorComponent>(entt);
                    }
                    if (renderable.GetName().find("_INTERACTABLE_") != std::string::npos)
                    {
                        destination->emplace<DialogComponent>(entt);
                    }
                    if (renderable.GetName().find("_CHEST_") != std::string::npos)
                    {
                        destination->emplace<InventoryComponent>(entt);
                    }
                }
            });

        std::cout << "FINISH: Loading map data from file." << std::endl;
    }
} // namespace lq::maploader
