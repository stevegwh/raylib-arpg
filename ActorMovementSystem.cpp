//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ActorMovementSystem.hpp"
#include "Game.hpp"

namespace sage
{

    void ActorMovementSystem::onCursorClick()
    {
        if (cursor->collision.hit)
        {
            switch (Game::GetInstance().collisionSystem->GetComponent(cursor->rayCollisionResultInfo.collidedEntityId)->collisionLayer)
            {
            case FLOOR:
                //Game::GetInstance().transformSystem->MoveToLocation(playerId, cursor->collision.point);
                Vector2 idx = Game::GetInstance().navigationGridSystem->WorldToGridSpace(cursor->collision.point);
                std::cout << "x: " << idx.x << " y: " << idx.y << std::endl;
                auto playerPos = Game::GetInstance().transformSystem->GetComponent(playerId);
                auto path = Game::GetInstance().navigationGridSystem->Pathfind(playerPos->position, cursor->collision.point);
                
                //std::vector<Vector3> path = { {42, 0, 4}, {10, 0, 44}, { -50, 0, -50} };
                Game::GetInstance().transformSystem->PathfindToLocation(playerId, path);
            }
        }
    }
} // sage