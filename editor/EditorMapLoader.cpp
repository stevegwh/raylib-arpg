#include "EditorMapLoader.hpp"

#include "EditorComponents.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Light.hpp"
#include "engine/Serializer.hpp"

#include "cereal/types/vector.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace sage::editor
{
    namespace
    {
        constexpr char kEditorLayoutMapMagic[4] = {'L', 'Q', 'E', '1'};

        struct LayoutEntityRecord
        {
            sage::serializer::entity entity{};
            sgTransform transform{};
            Collideable collideable{};
            Renderable renderable{};

            template <class Archive>
            void serialize(Archive& archive)
            {
                archive(entity, transform, collideable, renderable);
            }
        };
    } // namespace

    bool IsEditorLayoutMap(const char* path)
    {
        std::ifstream storage(path, std::ios::binary);
        if (!storage.is_open()) return false;

        char fileMagic[4]{};
        storage.read(fileMagic, sizeof(fileMagic));
        return storage.gcount() == sizeof(fileMagic) &&
               std::memcmp(fileMagic, kEditorLayoutMapMagic, sizeof(fileMagic)) == 0;
    }

    bool LoadMap(entt::registry* destination, const char* path)
    {
        assert(destination != nullptr);
        if (!IsEditorLayoutMap(path))
        {
            std::cerr << "ERROR: Not an editor layout map: " << path << std::endl;
            return false;
        }

        std::cout << "START: Loading layout map data from file (editor)." << std::endl;

        std::unordered_map<std::uint32_t, entt::entity> idMap;

        sage::serializer::ReadCompressedBinary(
            path, kEditorLayoutMapMagic, [&](cereal::BinaryInputArchive& input, std::istream&) {
                std::vector<Light> lights;
                input(lights);
                for (const auto& light : lights)
                {
                    const auto entity = destination->create();
                    auto loadedLight = light;
                    loadedLight.enabled = true;
                    destination->emplace<EditorMapEntity>(entity);
                    destination->emplace<Light>(entity, loadedLight);
                }

                std::vector<LayoutEntityRecord> layoutEntities;
                input(layoutEntities);
                for (const auto& record : layoutEntities)
                {
                    const auto entity = destination->create();
                    destination->emplace<EditorMapEntity>(entity);
                    destination->emplace<sgTransform>(entity, record.transform);
                    destination->emplace<Collideable>(entity, record.collideable);
                    destination->get<Collideable>(entity).isStatic = true;
                    destination->emplace<Renderable>(entity, record.renderable);
                    idMap[record.entity.id] = entity;
                }
            });

        for (auto view = destination->view<EditorMapEntity, sgTransform>(); const auto entity : view)
        {
            view.get<sgTransform>(entity).ResolveSerializedParent(idMap);
        }

        std::cout << "FINISH: Loading layout map data from file (editor)." << std::endl;
        return true;
    }

    void SaveMap(entt::registry& source, const char* path)
    {
        std::cout << "START: Saving layout map data to file (editor)." << std::endl;

        const std::filesystem::path outputPath{path};
        if (const auto parent = outputPath.parent_path(); !parent.empty())
        {
            std::filesystem::create_directories(parent);
        }

        sage::serializer::WriteCompressedBinary(
            path, kEditorLayoutMapMagic, [&](cereal::BinaryOutputArchive& output) {
                std::vector<Light> lights;
                for (const auto entity : source.view<EditorMapEntity, Light>())
                {
                    auto light = source.get<Light>(entity);
                    if (source.any_of<sgTransform>(entity))
                    {
                        light.position = source.get<sgTransform>(entity).GetWorldPos();
                    }
                    lights.push_back(light);
                }
                output(lights);

                std::vector<LayoutEntityRecord> layoutEntities;
                const auto view = source.view<EditorMapEntity, sgTransform, Renderable, Collideable>();
                for (const auto entityHandle : view)
                {
                    auto& record = layoutEntities.emplace_back();
                    record.entity.id = entt::entt_traits<entt::entity>::to_entity(entityHandle);
                    record.transform = view.get<sgTransform>(entityHandle);
                    record.collideable = view.get<Collideable>(entityHandle);
                    record.renderable = view.get<Renderable>(entityHandle);
                }
                output(layoutEntities);
            });

        std::cout << "FINISH: Saving layout map data to file (editor)." << std::endl;
    }
} // namespace sage::editor
