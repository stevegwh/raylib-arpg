#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <filesystem>
#include <string>
#include <vector>

namespace sage::editor
{
    struct FlatpackCatalogEntry
    {
        std::string displayName;
        std::filesystem::path path;
    };

    bool IsFlatpackFile(const char* path);

    // Walks the transform subtree rooted at `root` and writes a self-contained
    // flatpack archive. The saved root has position (0,0,0) so each instance
    // anchors at the cursor on placement; descendants keep their offsets from
    // the root. Returns true on success.
    bool SaveFlatpack(entt::registry& source, entt::entity root, const char* path);

    // Instantiates a flatpack from disk into `destination`. Every entity in the
    // archive is created fresh; parent links are restored using internal local
    // ids so each placement is independent of the saved template. Returns the
    // root entity (entt::null on failure).
    entt::entity LoadFlatpack(
        entt::registry& destination, const char* path, Vector3 anchorWorldPos);

    // Returns absolute paths of every *.bin file in `directory` that begins
    // with the flatpack magic header. Missing directories yield an empty list.
    [[nodiscard]] std::vector<FlatpackCatalogEntry> ListFlatpacks(const std::filesystem::path& directory);
} // namespace sage::editor
