//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "KeyMapping.hpp"
#include "Application.hpp"
#include "Gui.hpp"

namespace sage
{

class Editor : public Application
{
    void init() override;
    void draw() override;
    void drawGrid(bool drawDebug);
    void enablePlayMode();
    void enableEditMode();
    void manageScenes();
public:
    void Update() override;
};

} // sage
