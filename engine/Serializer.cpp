//
// Created by Steve Wheeler on 21/03/2024.
//

#include "Serializer.hpp"

#include "components/Collideable.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"

#include <unordered_map>

namespace sage::serializer
{
    // ----------------------------------------------

    void LoadAssetBinFile(entt::registry* destination, const char* path)
    {
        assert(destination != nullptr);
        std::cout << "START: Loading asset bin." << std::endl;

        std::unordered_map<std::uint32_t, entt::entity> idMap;

        ReadCompressedBinary(path, kAssetBinMagic, [&](cereal::BinaryInputArchive& input, std::istream& stream) {
            input(ResourceManager::GetInstance());

            // Not necessary for asset bin?
            while (stream.peek() != EOF)
            {
                entity entityId{};
                auto entt = destination->create();
                auto& transform = destination->emplace<sgTransform>(entt);
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
                idMap[entityId.id] = entt;
            }
        });

        for (auto [e, t] : destination->view<sgTransform>().each())
        {
            t.ResolveSerializedParent(idMap);
        }

        std::cout << "FINISH: Loading asset bin." << std::endl;
    }
} // namespace sage::serializer
