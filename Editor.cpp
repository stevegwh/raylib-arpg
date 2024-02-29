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
                if (currentEditorMode == CREATE)
                {
                    Game::GetInstance().createTower(cursor->collision.point, "Tower Instance");
                }
                else if (currentEditorMode == SELECT)
                {
                    moveSelectedObjectToCursorHit();
                    selectedObject = 0;
                    currentEditorMode = IDLE;
                }
                break;
            case BUILDING:
                currentEditorMode = SELECT;
                selectedObject = cursor->rayCollisionResultInfo.collidedEntityId;
                break;
            }
        }
        else
        {
            selectedObject = 0;
        }
    }

    void Editor::OnDeleteModeKeyPressed()
    {
        if (currentEditorMode != SELECT) return;
        Game::GetInstance().removeTower(selectedObject);
        selectedObject = 0;
        currentEditorMode = IDLE;
    }

    void Editor::OnCreateModeKeyPressed()
    {
        if (currentEditorMode == CREATE) currentEditorMode = IDLE;
        else currentEditorMode = CREATE;
    }
    
    void Editor::OnGenGridKeyPressed()
    {
        Game::GetInstance().navigationGridSystem->FindOccupiedGridSquares();
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
        if (currentEditorMode == SELECT)
        {
            Game::GetInstance().collisionSystem->BoundingBoxDraw(selectedObject, ORANGE);
        }
    }


    void Editor::DrawDebugText()
    {
        std::string mode = "NONE";
        if (currentEditorMode == IDLE) mode = "IDLE";
        else if (currentEditorMode == SELECT) mode = "SELECT";
        else if (currentEditorMode == MOVE) mode = "MOVE";
        else if (currentEditorMode == CREATE) mode = "CREATE";

        DrawText(TextFormat("Editor Mode: %s", mode.c_str()), SCREEN_WIDTH - 150, 50, 10, BLACK);
    }
}
