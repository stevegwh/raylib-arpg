//
// Created by steve on 22/02/2024.
//

#pragma once

#include "UserInput.hpp"
#include "EventManager.hpp"
#include "GameManager.hpp"
#include "Scene.hpp"

#include <vector>
#include <memory>

namespace sage
{

enum EditorMode
{
    IDLE,
    SELECT,
    MOVE,
    CREATE
};

class Editor : public Scene
{
    EditorMode currentEditorMode = IDLE;
    UserInput* cursor;
    
    // Event responses
    void OnCursorClick();
    void OnCollisionHit();
    void OnSerializeButton();
    void OnDeleteModeKeyPressed();
    void OnCreateModeKeyPressed();
    void OnGenGridKeyPressed();
    void OnRunModePressed();
    
    EntityID selectedObject{};
    void moveSelectedObjectToCursorHit();
    
    std::unique_ptr<EventManager> eventManager;
    
public:
    explicit Editor(UserInput* _cursor);
    ~Editor() override;
    void Update() override;
    void Draw3D() override;
    void Draw2D() override;
};

}



