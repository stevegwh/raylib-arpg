//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ActorMovementSystem.hpp"
#include "../Application.hpp"

namespace sage
{

void ActorMovementSystem::PathfindToLocation(entt::entity id)
{
    {
        // If mouse clicks outside of bounds, then return
        Vector2 tmp;
        if (!navigationGridSystem->WorldToGridSpace(cursor->collision.point, tmp)) return;
    }
    const auto& actor = registry->get<Actor>(id);
    Vector2 minRange;
    Vector2 maxRange;
    navigationGridSystem->GetPathfindRange(id, actor.pathfindingBounds, minRange, maxRange);
    navigationGridSystem->DrawDebugPathfinding(minRange, maxRange);

    const auto& playerPos = registry->get<Transform>(id);
    auto path = navigationGridSystem->Pathfind(playerPos.position, cursor->collision.point, minRange, maxRange);
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
    if (cursor->collision.hit)
    {
        switch (registry->get<Collideable>(cursor->rayCollisionResultInfo.collidedEntityId).collisionLayer)
        {
        case CollisionLayer::FLOOR:
            PathfindToLocation(controlledActorId);
            //MoveToLocation(controlledActorId);
        }
    }
}
    
void ActorMovementSystem::SetControlledActor(entt::entity id)
{
    onControlledActorChange.publish(id);
    controlledActorId = id;
    {
        entt::sink onClick{userInput->onClickEvent};
        onClick.connect<&ActorMovementSystem::onCursorClick>(this);
    }
}

entt::entity ActorMovementSystem::GetControlledActor()
{
    return controlledActorId;
}

ActorMovementSystem::ActorMovementSystem(entt::registry* _registry, 
                                         Cursor* _cursor,
                                         UserInput* _userInput,
                                         NavigationGridSystem* _navigationGridSystem, 
                                         TransformSystem* _transformSystem) :
    BaseSystem<Actor>(_registry), cursor(_cursor), userInput(_userInput),
    navigationGridSystem(_navigationGridSystem), transformSystem(_transformSystem)
{

}

} // sage