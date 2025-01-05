#include "ControllableActor.hpp"

#include "TextureTerrainOverlay.hpp" // used for construction

namespace sage
{

    ControllableActor::ControllableActor()
        : cursorOnEnemyLeftClickCnx(std::make_shared<Connection<entt::entity>>()),
          cursorOnEnemyRightClickCnx(std::make_shared<Connection<entt::entity>>()),
          cursorOnFloorClickCnx(std::make_shared<Connection<entt::entity>>()),
          cursorOnNPCLeftClickCnx(std::make_shared<Connection<entt::entity>>()),
          onEnemyLeftClickCnx(std::make_shared<Connection<entt::entity, entt::entity>>()),
          onEnemyRightClickCnx(std::make_shared<Connection<entt::entity, entt::entity>>()),
          onFloorClickCnx(std::make_shared<Connection<entt::entity, entt::entity>>()),
          onNPCLeftClickCnx(std::make_shared<Connection<entt::entity, entt::entity>>()),
          onEnemyLeftClick(std::make_unique<Event<entt::entity, entt::entity>>()),
          onEnemyRightClick(std::make_unique<Event<entt::entity, entt::entity>>()),
          onFloorClick(std::make_unique<Event<entt::entity, entt::entity>>()),
          onNPCLeftClick(std::make_unique<Event<entt::entity, entt::entity>>())
    {
    }
} // namespace sage