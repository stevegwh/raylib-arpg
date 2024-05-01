//
// Created by steve on 22/02/2024.
//

#pragma once

#include "UserInput.hpp"
#include "EventManager.hpp"
#include "GameManager.hpp"
#include "Scene.hpp"

#include <entt/entt.hpp>

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
    entt::registry* registry;
    UserInput* cursor;
    
    // Event responses
    void OnCursorClick();
    void OnCollisionHit();
    void OnSerializeButton();
    void OnDeleteModeKeyPressed();
    void OnCreateModeKeyPressed();
    void OnGenGridKeyPressed();
    void OnRunModePressed();
    
    entt::entity selectedObject{};
    void moveSelectedObjectToCursorHit();
    
    std::unique_ptr<EventManager> eventManager;
    
public:
    Editor(entt::registry* _registry, UserInput* _cursor);
    ~Editor() override;
    void Update() override;
    void Draw3D() override;
    void Draw2D() override;
};

}



