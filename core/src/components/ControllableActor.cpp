#include "ControllableActor.hpp"

#include "TextureTerrainOverlay.hpp" // used for construction

namespace sage
{

    ControllableActor::ControllableActor()
        : onEnemyLeftClick(std::make_unique<Event<entt::entity, entt::entity>>()),
          onEnemyRightClick(std::make_unique<Event<entt::entity, entt::entity>>()),
          onFloorClick(std::make_unique<Event<entt::entity, entt::entity>>()),
          onNPCLeftClick(std::make_unique<Event<entt::entity, entt::entity>>())
    {
    }
} // namespace sage