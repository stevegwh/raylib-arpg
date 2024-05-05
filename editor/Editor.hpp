//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "GameManager.hpp"

namespace sage
{

class Editor : public GameManager
{
    void init() override;
    void draw() override;
    void drawGrid();
    void enablePlayMode();
    void enableEditMode();
    void manageScenes();
public:
    void Update() override;
};

} // sage
