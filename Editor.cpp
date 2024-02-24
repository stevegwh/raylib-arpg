//
// Created by steve on 22/02/2024.
//

#include "Editor.hpp"

// NB: "Game" is friend
#include "Game.hpp"

#include <iostream>

namespace sage
{
    void Editor::moveSelectedObjectToCursorHit()
    {
        Transform newTransform(selectedObject);
        newTransform.position = cursor->collision.point;

        const Renderable* renderable = Game::GetInstance().renderSystem->GetComponent(selectedObject);

        BoundingBox bb;
        bb.min = Vector3Add(renderable->meshBoundingBox.min, newTransform.position);
        bb.max = Vector3Add(renderable->meshBoundingBox.max, newTransform.position);
        Game::GetInstance().transformSystem->SetComponent(selectedObject, newTransform);
        Game::GetInstance().collisionSystem->SetBoundingBox(selectedObject, bb);
        
    }
    
    void Editor::OnCursorClick()
    {
        if (cursor->collision.hit)
        {
            switch (Game::GetInstance().collisionSystem->GetComponent(cursor->rayCollisionResultInfo.collidedEntityId)->collisionLayer)
            {
            case DEFAULT:
                break;
            case FLOOR:
                if (selectedObject == 0)
                {
                    Game::GetInstance().createTower(cursor->collision.point, "Tower Instance");
                }
                else
                {
                    moveSelectedObjectToCursorHit();
                    selectedObject = 0;
                }
                break;
            case BUILDING:
                
                Game::GetInstance().removeTower(cursor->rayCollisionResultInfo.collidedEntityId);
                //selectedObject = cursor->rayCollisionResultInfo.collidedEntityId;
                break;
            }
        }
        else
        {
            selectedObject = 0;
        }
    }

    void Editor::OnCollisionHit()
    {
        //std::cout << "Collision detected. \n";
        //    if (colSystem.GetComponent(rayCollisionResultInfo.collidedEntityId).collisionLayer == FLOOR)
        //    {
        //        // Place model
        //    }
        //    else
        //    {
        //        // Select model
        //        // Store entityID of selected model
        //        // Change bounding box colour
        //    }
    }

    void Editor::Draw()
    {
        if (selectedObject > 0)
        {
            Game::GetInstance().collisionSystem->BoundingBoxDraw(selectedObject, ORANGE);
        }
    }
}
