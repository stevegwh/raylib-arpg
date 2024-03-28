//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once


#include "State.hpp"
#include "UserInput.hpp"

namespace sage
{

class Game : public State
{
    UserInput* cursor;
    void onEditorModePressed();
    std::unordered_map<std::string, std::shared_ptr<EventCallback>> eventCallbacks;
public:
    
    Game(sage::UserInput* _cursor);
    ~Game() override;
    void Update() override;
    void Draw3D() override;
    void Draw2D() override;
};

} // sage
