//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once


#include "Scene.hpp"
#include "UserInput.hpp"
#include "EventManager.hpp"

#include <vector>
#include <memory>

namespace sage
{

class Game : public Scene
{
    UserInput* cursor;
    void onEditorModePressed();
    std::unique_ptr<EventManager> eventManager;
public:
    
    explicit Game(sage::UserInput* _cursor);
    ~Game() override;
    void Update() override;
    void Draw3D() override;
    void Draw2D() override;
};

} // sage
