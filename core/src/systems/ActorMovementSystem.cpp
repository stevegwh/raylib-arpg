//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ActorMovementSystem.hpp"
#include "../GameManager.hpp"

namespace sage
{


void ActorMovementSystem::PathfindToLocation(entt::entity id)
{
    {
        Vector2 tmp;
        if (!navigationGridSystem->WorldToGridSpace(cursor->collision.point, tmp)) return;
    }

    for (int i = 0; i  < navigationGridSystem->gridSquares.size(); i++)
    {
        for (int j = 0; j < navigationGridSystem->gridSquares.at(0).size(); j++)
        {
            navigationGridSystem->gridSquares[i][j]->debugColor = false;
        }
    }
    
    Vector2 minRange;
    Vector2 maxRange;
    
    navigationGridSystem->GetPathfindRange(id, 25, minRange, maxRange);
    
//    navigationGridSystem->WorldToGridSpace({-25,0,-25}, minRange);
//    navigationGridSystem->WorldToGridSpace({25,0,25}, maxRange);

    for (int i = minRange.y; i  < maxRange.y; i++) 
    {
        for (int j = minRange.x; j < maxRange.x; j++)
        {
            navigationGridSystem->gridSquares[i][j]->debugColor = true;
        }
    }
//    navigationGridSystem->GetPathfindRange(id, 25, minRange, maxRange);

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
        //ECS->collisionSystem->GetComponent(cursor->rayCollisionResultInfo.collidedEntityId)->collisionLayer

        switch (registry->get<Collideable>(cursor->rayCollisionResultInfo.collidedEntityId).collisionLayer)
        {
        case FLOOR:
            PathfindToLocation(controlledActorId);
            //MoveToLocation(controlledActorId);
        }
    }
}
    
void ActorMovementSystem::SetControlledActor(entt::entity id)
{
    controlledActorId = id;
    userInput->dOnClickEvent.connect<&ActorMovementSystem::onCursorClick>(this);
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