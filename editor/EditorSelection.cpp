#include "EditorSelection.hpp"

#include "engine/EngineSystems.hpp"
#include "engine/components/sgTransform.hpp"

namespace sage::editor
{
    EditorSelection::EditorSelection(EngineSystems* _sys) : sys(_sys)
    {
    }

    bool EditorSelection::Select(const entt::entity entity)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return false;
        selectedEntity = entity;
        return true;
    }

    void EditorSelection::Clear()
    {
        selectedEntity.reset();
    }

    bool EditorSelection::HasSelection() const
    {
        return selectedEntity.has_value();
    }

    std::optional<entt::entity> EditorSelection::Current() const
    {
        return selectedEntity;
    }

    std::optional<entt::entity> EditorSelection::ActiveTransformEntity() const
    {
        if (!selectedEntity.has_value()) return std::nullopt;
        if (!sys->registry->valid(*selectedEntity) || !sys->registry->any_of<sgTransform>(*selectedEntity))
        {
            return std::nullopt;
        }
        return selectedEntity;
    }
} // namespace sage::editor
