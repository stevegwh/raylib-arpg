//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "State.hpp"

namespace sage
{

class Game : State
{
public:
    Game();
    ~Game() override;
    void Update() override;
    void Draw() override;
};

} // sage
