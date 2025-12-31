//
// Created by Steve Wheeler on 21/03/2024.
//

#include "Serializer.hpp"

#include "components/Collideable.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"

namespace sage::serializer
{
    // ----------------------------------------------

    void LoadAssetBinFile(entt::registry* destination, const char* path)
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
} // namespace sage::serializer
