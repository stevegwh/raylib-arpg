//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ActorMovementSystem.hpp"
#include "../Application.hpp"

namespace sage
{

void ActorMovementSystem::PathfindToLocation(entt::entity id, Vector3 location)
{
    {
        // If mouse clicks outside of bounds, then return
        Vector2 tmp;
        if (!navigationGridSystem->WorldToGridSpace(location, tmp)) return;
    }
    const auto& actor = registry->get<Actor>(id);
    Vector2 minRange;
    Vector2 maxRange;
    navigationGridSystem->GetPathfindRange(id, actor.pathfindingBounds, minRange, maxRange);
    navigationGridSystem->DrawDebugPathfinding(minRange, maxRange);

    const auto& playerPos = registry->get<Transform>(id);
    auto path = navigationGridSystem->Pathfind(playerPos.position, location, minRange, maxRange);
    if (!path.empty()) transformSystem->PathfindToLocation(id, path);
}

void ActorMovementSystem::MoveToLocation(entt::entity id)
{
    transformSystem->PathfindToLocation(id, {cursor->collision.point});
}

void ActorMovementSystem::PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol)
{
    transformSystem->PathfindToLocation(id, patrol);
}

void ActorMovementSystem::onCursorClick()
{
    PathfindToLocation(controlledActorId, cursor->collision.point);
}
    
void ActorMovementSystem::SetControlledActor(entt::entity id)
{
    onControlledActorChange.publish(id);
    controlledActorId = id;
}

entt::entity ActorMovementSystem::GetControlledActor()
{
    return controlledActorId;
}

void ActorMovementSystem::Enable()
{
    entt::sink onClick{cursor->onFloorClick};
    onClick.connect<&ActorMovementSystem::onCursorClick>(this);
}
void ActorMovementSystem::Disable()
{
    entt::sink onClick{cursor->onFloorClick};
    onClick.disconnect<&ActorMovementSystem::onCursorClick>(this);
}

ActorMovementSystem::ActorMovementSystem(entt::registry* _registry, 
                                         Cursor* _cursor,
                                         UserInput* _userInput,
                                         NavigationGridSystem* _navigationGridSystem, 
                                         TransformSystem* _transformSystem) :
    BaseSystem<Actor>(_registry), cursor(_cursor), userInput(_userInput),
    navigationGridSystem(_navigationGridSystem), transformSystem(_transformSystem)
{}

} // sage