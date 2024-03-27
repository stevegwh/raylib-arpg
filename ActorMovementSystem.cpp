//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ActorMovementSystem.hpp"
#include "GameManager.hpp"

namespace sage
{

    void ActorMovementSystem::onCursorClick()
    {
        if (cursor->collision.hit)
        {
            switch (GM.collisionSystem->GetComponent(cursor->rayCollisionResultInfo.collidedEntityId)->collisionLayer)
            {
            case FLOOR:
                //GM.transformSystem->MoveToLocation(actorId, cursor->collision.point);
                Vector2 idx = GM.navigationGridSystem->WorldToGridSpace(cursor->collision.point);
                std::cout << "x: " << idx.x << " y: " << idx.y << std::endl;
                auto playerPos = GM.transformSystem->GetComponent(actorId);
                auto path = GM.navigationGridSystem->Pathfind(playerPos->position, cursor->collision.point);
                
                //std::vector<Vector3> path = { {42, 0, 4}, {10, 0, 44}, { -50, 0, -50} };
                GM.transformSystem->PathfindToLocation(actorId, path);
            }
        }
    }
} // sage