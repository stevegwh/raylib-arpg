#pragma once

#include "entt/entt.hpp"

namespace sage::editor
{
    // Loads/saves the editor-only layout map format. This is intentionally
    // separate from the game/respacker map .bin format.
    [[nodiscard]] bool IsEditorLayoutMap(const char* path);
    bool LoadMap(entt::registry* destination, const char* path);
    void SaveMap(entt::registry& source, const char* path);
} // namespace sage::editor
