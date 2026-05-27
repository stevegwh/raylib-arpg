#include "EditorFlatpack.hpp"

#include "EditorComponents.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Light.hpp"
#include "engine/Serializer.hpp"

#include "cereal/types/vector.hpp"
#include "raymath.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <unordered_map>

namespace sage::editor
{
    namespace
    {
        constexpr char kFlatpackMagic[4] = {'L', 'Q', 'F', 'P'};

        // A serialized entity from the source registry. parentLocalId points into
        // this same vector (-1 for the root). Transform values are captured as
        // plain Vector3 (rather than sgTransform itself) so saving doesn't have
        // to touch the registry-bound proxies on the source transforms.
        // Component blobs are flagged so we can leave optional components empty
        // when the source entity doesn't carry them.
        struct FlatpackEntityRecord
        {
            std::int32_t parentLocalId = -1;
            Vector3 worldPos{};
            Vector3 worldRot{};
            Vector3 worldScale{1.0f, 1.0f, 1.0f};
            bool hasCollideable = false;
            Collideable collideable{};
            bool hasRenderable = false;
            Renderable renderable{};
            bool hasLight = false;
            Light light{};

            template <class Archive>
            void serialize(Archive& archive)
            {
                archive(parentLocalId, worldPos, worldRot, worldScale);
                archive(hasCollideable);
                if (hasCollideable) archive(collideable);
                archive(hasRenderable);
                if (hasRenderable) archive(renderable);
                archive(hasLight);
                if (hasLight) archive(light);
            }
        };
    } // namespace

    bool IsFlatpackFile(const char* path)
    {
        std::ifstream storage(path, std::ios::binary);
        if (!storage.is_open()) return false;

        char fileMagic[4]{};
        storage.read(fileMagic, sizeof(fileMagic));
        return storage.gcount() == sizeof(fileMagic) &&
               std::memcmp(fileMagic, kFlatpackMagic, sizeof(fileMagic)) == 0;
    }

    bool SaveFlatpack(entt::registry& source, entt::entity root, const char* path)
    {
        if (!source.valid(root) || !source.any_of<sgTransform>(root))
        {
            std::cerr << "ERROR: SaveFlatpack: root is not a valid sgTransform entity." << std::endl;
            return false;
        }

        // Depth-first walk of the source subtree. Each entry maps a source entity
        // to its local id (index in the records vector).
        std::vector<entt::entity> subtreeOrder;
        std::unordered_map<entt::entity, std::int32_t> localIds;
        auto visit = [&](auto& self, entt::entity entity) -> void {
            if (!source.valid(entity) || !source.any_of<sgTransform>(entity)) return;
            localIds.emplace(entity, static_cast<std::int32_t>(subtreeOrder.size()));
            subtreeOrder.push_back(entity);
            for (const auto child : source.get<sgTransform>(entity).GetChildren())
            {
                self(self, child);
            }
        };
        visit(visit, root);

        const Vector3 rootWorldOrigin = source.get<sgTransform>(root).GetWorldPos();

        std::vector<FlatpackEntityRecord> records;
        records.reserve(subtreeOrder.size());
        for (const auto entity : subtreeOrder)
        {
            auto& record = records.emplace_back();
            const auto& transform = source.get<sgTransform>(entity);
            const auto parentIter = localIds.find(transform.GetParent());
            record.parentLocalId = (parentIter != localIds.end()) ? parentIter->second : -1;

            // Rebase world position relative to the root so the saved root sits
            // at the origin and descendants keep their world-space offsets.
            record.worldPos = Vector3Subtract(transform.GetWorldPos(), rootWorldOrigin);
            record.worldRot = transform.GetWorldRot();
            record.worldScale = transform.GetScale();

            if (source.any_of<Collideable>(entity))
            {
                record.hasCollideable = true;
                record.collideable = source.get<Collideable>(entity);
            }
            if (source.any_of<Renderable>(entity))
            {
                record.hasRenderable = true;
                record.renderable = source.get<Renderable>(entity);
            }
            if (source.any_of<Light>(entity))
            {
                record.hasLight = true;
                record.light = source.get<Light>(entity);
                // Light::position is a world-space cache; rebase the same way
                // so the saved root sits at the origin.
                record.light.position = Vector3Subtract(record.light.position, rootWorldOrigin);
            }
        }

        const std::filesystem::path outputPath{path};
        if (const auto parent = outputPath.parent_path(); !parent.empty())
        {
            std::filesystem::create_directories(parent);
        }

        sage::serializer::WriteCompressedBinary(
            path, kFlatpackMagic, [&](cereal::BinaryOutputArchive& output) { output(records); });

        return true;
    }

    entt::entity LoadFlatpack(
        entt::registry& destination, const char* path, const Vector3 anchorWorldPos)
    {
        if (!IsFlatpackFile(path))
        {
            std::cerr << "ERROR: Not a flatpack file: " << path << std::endl;
            return entt::null;
        }

        std::vector<FlatpackEntityRecord> records;
        sage::serializer::ReadCompressedBinary(
            path, kFlatpackMagic, [&](cereal::BinaryInputArchive& input, std::istream&) { input(records); });
        if (records.empty()) return entt::null;

        // Create entities up-front so parent local ids resolve to real entt::entity values.
        std::vector<entt::entity> created;
        created.reserve(records.size());
        for (std::size_t i = 0; i < records.size(); ++i)
        {
            created.push_back(destination.create());
        }

        // Apply transforms top-down so each call to SetParent sees the parent
        // already positioned in world space. We saved records in DFS order with
        // the root at index 0, so iterating in order is top-down.
        for (std::size_t i = 0; i < records.size(); ++i)
        {
            const auto& record = records[i];
            const auto entity = created[i];

            destination.emplace<sgTransform>(entity);
            // emplace fires on_construct which binds the transform to the system,
            // so the proxy assignments below route through TransformSystem.
            auto& transform = destination.get<sgTransform>(entity);
            transform.position.world = Vector3Add(record.worldPos, anchorWorldPos);
            transform.rotation.world = record.worldRot;
            transform.scale.world = record.worldScale;

            if (record.parentLocalId >= 0 &&
                static_cast<std::size_t>(record.parentLocalId) < created.size())
            {
                transform.SetParent(created[static_cast<std::size_t>(record.parentLocalId)]);
            }
        }

        // Restore optional components after the hierarchy is in place. Renderable
        // emplaces re-load the model from its asset key via the ResourceManager
        // (see Renderable::load), so each instance gets its own model handle.
        for (std::size_t i = 0; i < records.size(); ++i)
        {
            const auto& record = records[i];
            const auto entity = created[i];

            destination.emplace<EditorMapEntity>(entity);
            destination.emplace<EditorObjectDescriptor>(
                entity,
                EditorObjectDescriptor{
                    .name = std::format("entity_{}", entt::to_integral(entity)),
                    .category = record.hasRenderable ? "Flatpack" : "FlatpackNode",
                    .selectable = true,
                    .visibleInHierarchy = true,
                    .locked = false});

            if (record.hasCollideable)
            {
                destination.emplace<Collideable>(entity, record.collideable);
            }
            if (record.hasRenderable)
            {
                destination.emplace<Renderable>(entity, record.renderable);
            }
            if (record.hasLight)
            {
                auto light = record.light;
                // Rebase the cached world position from the flatpack-root frame
                // (which was 0) into the destination world frame.
                light.position = Vector3Add(light.position, anchorWorldPos);
                destination.emplace<Light>(entity, light);
            }
        }

        return created.front();
    }

    std::vector<FlatpackCatalogEntry> ListFlatpacks(const std::filesystem::path& directory)
    {
        std::vector<FlatpackCatalogEntry> entries;
        if (!std::filesystem::is_directory(directory)) return entries;

        for (const auto& dirEntry : std::filesystem::directory_iterator{directory})
        {
            if (!dirEntry.is_regular_file()) continue;
            const auto& path = dirEntry.path();
            if (path.extension() != ".flatpack") continue;
            if (!IsFlatpackFile(path.string().c_str())) continue;
            entries.push_back({.displayName = path.stem().string(), .path = path});
        }

        std::sort(entries.begin(), entries.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.displayName < rhs.displayName;
        });
        return entries;
    }
} // namespace sage::editor
