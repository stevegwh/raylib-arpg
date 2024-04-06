//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ActorMovementSystem.hpp"
#include "GameManager.hpp"

namespace sage
{


void ActorMovementSystem::PathfindToLocation(EntityID id)
{
    //GM.transformSystem->MoveToLocation(controlledActorId, cursor->collision.point);
    Vector2 idx = ECS->navigationGridSystem->WorldToGridSpace(cursor->collision.point);
    //std::cout << "x: " << idx.x << " y: " << idx.y << std::endl;
    auto playerPos = ECS->transformSystem->GetComponent(id);
    auto path = ECS->navigationGridSystem->Pathfind(playerPos->position, cursor->collision.point);

    //std::vector<Vector3> path = { {42, 0, 4}, {10, 0, 44}, { -50, 0, -50} };
    ECS->transformSystem->PathfindToLocation(id, path);
}

void ActorMovementSystem::MoveToLocation(EntityID id)
{
    ECS->transformSystem->PathfindToLocation(id, {cursor->collision.point});
}

void ActorMovementSystem::PatrolLocations(EntityID id, const std::vector<Vector3>& patrol)
{
    ECS->transformSystem->PathfindToLocation(id, patrol);
}

void ActorMovementSystem::onCursorClick()
{
    if (cursor->collision.hit)
    {
        switch (ECS->collisionSystem->GetComponent(cursor->rayCollisionResultInfo.collidedEntityId)->collisionLayer)
        {
        case FLOOR:
            PathfindToLocation(controlledActorId);
            //MoveToLocation(controlledActorId);
        }
    }
}
    
void ActorMovementSystem::SetControlledActor(EntityID id)
{
    controlledActorId = id;
    eventManager->Subscribe([p = this] { p->onCursorClick(); }, *cursor->OnClickEvent);
}

} // sage