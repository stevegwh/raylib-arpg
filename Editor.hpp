//
// Created by steve on 22/02/2024.
//

#pragma once

#include "UserInput.hpp"
#include "EventCallback.hpp"
#include "GameManager.hpp"
#include "State.hpp"

#include <unordered_map>

namespace sage
{

enum EditorMode
{
    IDLE,
    SELECT,
    MOVE,
    CREATE
};

// NB: "GameManager" is friend
class Editor : public State
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
    
    std::unordered_map<std::string, std::shared_ptr<EventCallback>> eventCallbacks;
    
public:
    explicit Editor(UserInput* _cursor);
    ~Editor() override;
    void Update() override;
    void Draw3D() override;
    void Draw2D() override;
};

}



