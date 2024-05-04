//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ActorMovementSystem.hpp"
#include "../GameManager.hpp"

namespace sage
{


void ActorMovementSystem::PathfindToLocation(entt::entity id)
{

    Vector2 idx = navigationGridSystem->WorldToGridSpace(cursor->collision.point);
    const auto& playerPos = registry->get<Transform>(id);
    auto path = navigationGridSystem->Pathfind(playerPos.position, cursor->collision.point);

    //std::vector<Vector3> path = { {42, 0, 4}, {10, 0, 44}, { -50, 0, -50} };
    transformSystem->PathfindToLocation(id, path);
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