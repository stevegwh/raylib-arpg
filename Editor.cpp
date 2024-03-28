//
// Created by steve on 22/02/2024.
//

#include "Editor.hpp"

#include "GameObjectFactory.hpp"

// NB: "GameManager" is friend
#include "GameManager.hpp"

#include <iostream>

namespace sage
{
    Editor::Editor(UserInput* _cursor) : cursor(_cursor)
    {
        const std::function<void()> f1 = [p = this] { p->OnCursorClick(); };
        auto e1 = std::make_shared<EventCallback>(f1);
        eventCallbacks.push_back(e1);
        cursor->OnClickEvent->Subscribe(e1);

        const std::function<void()> f2 = [p = this] { p->OnCollisionHit(); };
        auto e2 = std::make_shared<EventCallback>(f2);
        eventCallbacks.push_back(e2);
        cursor->OnCollisionHitEvent->Subscribe(e2);

        const std::function<void()> f3 = [p = this] { p->OnDeleteModeKeyPressed(); };
        auto e3 = std::make_shared<EventCallback>(f3);
        eventCallbacks.push_back(e3);
        cursor->OnDeleteKeyPressedEvent->Subscribe(e3);

        const std::function<void()> f4 = [p = this] { p->OnCreateModeKeyPressed(); };
        auto e4 = std::make_shared<EventCallback>(f4);
        eventCallbacks.push_back(e4);
        cursor->OnCreateKeyPressedEvent->Subscribe(e4);

        const std::function<void()> f5 = [p = this] { p->OnGenGridKeyPressed(); };
        auto e5 = std::make_shared<EventCallback>(f5);
        eventCallbacks.push_back(e5);
        cursor->OnGenGridKeyPressedEvent->Subscribe(std::make_shared<EventCallback>(f5));

        const std::function<void()> f6 = [p = this] { p->OnSerializeButton(); };
        auto e6 = std::make_shared<EventCallback>(f6);
        eventCallbacks.push_back(e6);
        cursor->OnSerializeKeyPressedEvent->Subscribe(e6);

        const std::function<void()> f7 = [p = this] { p->OnRunModePressed(); };
        auto e7 = std::make_shared<EventCallback>(f7);
        eventCallbacks.push_back(e7);
        cursor->OnRunModePressedEvent->Subscribe(e7);

        EntityID floor = Registry::GetInstance().CreateEntity();
        Vector3 g0 = (Vector3){ -50.0f, 0.1f, -50.0f };
        Vector3 g2 = (Vector3){  50.0f, 0.1f,  50.0f };
        BoundingBox bb = {
            .min = g0,
            .max = g2
        };
        auto floorCollidable = std::make_unique<Collideable>(floor, bb);
        floorCollidable->collisionLayer = FLOOR;
        ECS->collisionSystem->AddComponent(std::move(floorCollidable));

//        auto floorWorldObject = std::make_unique<WorldObject>(floor);
//        worldSystem->AddComponent(std::move(floorWorldObject));

        ECS->DeserializeMap(); // TODO: Should specify path to saved map of scene
        // This should also be based on scene parameters
        ECS->navigationGridSystem->Init(100, 1.0f);
        ECS->navigationGridSystem->PopulateGrid();
    }
    
    Editor::~Editor()
    {
        
    }
    
    void Editor::moveSelectedObjectToCursorHit()
    {
        Transform newTransform(selectedObject);
        newTransform.position = cursor->collision.point;

        const Renderable* renderable = ECS->renderSystem->GetComponent(selectedObject);

        ECS->transformSystem->SetComponent(selectedObject, newTransform);
        //ECS->collisionSystem->UpdateWorldBoundingBox(selectedObject, newTransform.position);
        
    }
    
    void Editor::OnCursorClick()
    {
        if (cursor->collision.hit)
        {
            switch (ECS->collisionSystem->GetComponent(cursor->rayCollisionResultInfo.collidedEntityId)->collisionLayer)
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
        ECS->SerializeMap();
    }

    void Editor::OnDeleteModeKeyPressed()
    {
        if (currentEditorMode != SELECT) return;
        Registry::GetInstance().DeleteEntity(selectedObject);
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
        ECS->navigationGridSystem->PopulateGrid();
    }

    void Editor::OnRunModePressed()
    {
        GM.SetStateRun();
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
    
    void Editor::Update()
    {
        
    } 

    void Editor::Draw3D()
    {
        if (currentEditorMode == SELECT)
        {
            ECS->collisionSystem->BoundingBoxDraw(selectedObject, ORANGE);
        }

        ECS->renderSystem->Draw();

        DrawGrid(100, 1.0f);

        for (const auto& gridSquareRow : ECS->navigationGridSystem->GetGridSquares())
        {
            for (const auto& gridSquare : gridSquareRow)
            {
                BoundingBox bb;
                bb.min = gridSquare->worldPosMin;
                bb.max = gridSquare->worldPosMax;
                bb.max.y = 0.1f;
                Color color = gridSquare->occupied ? RED : GREEN;
                DrawBoundingBox(bb, color);
            }
        }
    }


    void Editor::Draw2D()
    {
        std::string mode = "NONE";
        if (currentEditorMode == IDLE) mode = "IDLE";
        else if (currentEditorMode == SELECT) mode = "SELECT";
        else if (currentEditorMode == MOVE) mode = "MOVE";
        else if (currentEditorMode == CREATE) mode = "CREATE";

        DrawText(TextFormat("Editor Mode: %s", mode.c_str()), SCREEN_WIDTH - 150, 50, 10, BLACK);
    }
}
