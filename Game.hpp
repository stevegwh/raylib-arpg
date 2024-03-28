//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once


#include "State.hpp"
#include "UserInput.hpp"

#include <vector>

namespace sage
{

class Game : public State
{
    UserInput* cursor;
    void onEditorModePressed();
    std::vector<std::shared_ptr<EventCallback>> eventCallbacks;
public:
    
    explicit Game(sage::UserInput* _cursor);
    ~Game() override;
    void Update() override;
    void Draw3D() override;
    void Draw2D() override;
};

} // sage
