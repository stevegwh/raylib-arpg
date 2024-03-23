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

        Game::GetInstance().transformSystem->SetComponent(selectedObject, newTransform);
        Game::GetInstance().collisionSystem->UpdateWorldBoundingBox(selectedObject, newTransform.position);
        
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
                    GameObjectFactory::createTower(cursor->collision.point, "Tower Instance");
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
    
    void Editor::OnSerializeButton()
    {
        std::vector<std::pair<std::string, std::vector<std::unordered_map<std::string, std::string>>>> serializeData;
        auto transforms = Game::GetInstance().transformSystem->SerializeComponents();
        auto renderables = Game::GetInstance().renderSystem->SerializeComponents();
        auto collideables = Game::GetInstance().collisionSystem->SerializeComponents();
        serializeData.push_back(transforms);
        serializeData.push_back(renderables);
        serializeData.push_back(collideables);
        Serializer::SerializeToFile(serializeData);
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
        Game::GetInstance().navigationGridSystem->PopulateGrid();
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
