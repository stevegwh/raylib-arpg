#include "Serializer.hpp"

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

namespace lq::serializer
{
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

            sage::ViewSerializer<sage::Spawner> spawnerLoader(&source);
            output(spawnerLoader);

            sage::ViewSerializer<sage::Light> lightLoader(&source);
            output(lightLoader);

            output(sage::ResourceManager::GetInstance());

            // TODO: Below would be solved better with ViewSerializer if we could pass in Renderable,
            // Collideable etc. Currently not possible as they cannot be copied.
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

            const auto view =
                source.view<sage::sgTransform, sage::Renderable, sage::Collideable>(entt::exclude<ItemComponent>);
            for (const auto& ent : view)
            {
                const auto& rend = view.get<sage::Renderable>(ent);
                const auto& trans = view.get<sage::sgTransform>(ent);
                const auto& col = view.get<sage::Collideable>(ent);

                sage::serializer::entity entity{};
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
                auto& transform = destination->emplace<sage::sgTransform>(entt, entt);
                auto& collideable = destination->emplace<sage::Collideable>(entt);
                auto& renderable = destination->emplace<sage::Renderable>(entt);
                auto& item = destination->emplace<ItemComponent>(entt);

                try
                {
                    input(entityId, transform, collideable, renderable, item);
                }
                catch (const cereal::Exception& e)
                {
                    std::cerr << "ERROR: Serialization error: " << e.what() << std::endl;
                    break;
                }
            }

            while (storage.peek() != EOF)
            {
                sage::serializer::entity entityId{}; // ignore this (old serialized entity)
                auto entt = destination->create();
                auto& transform = destination->emplace<sage::sgTransform>(entt, entt);
                auto& collideable = destination->emplace<sage::Collideable>(entt);
                auto& renderable = destination->emplace<sage::Renderable>(entt);

                try
                {
                    input(entityId, transform, collideable, renderable);
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
        }

        storage.close();
        std::cout << "FINISH: Loading resource data from file." << std::endl;
    }
} // namespace lq::serializer