//
// Created by Steve Wheeler on 09/04/2024.
//

#pragma once

#include "raylib.h"
#include "../rlights.h"

#include <vector>

namespace sage
{
typedef struct Renderable Renderable; //forward dec
class LightSubSystem
{
    std::vector<Renderable*> renderables;
public:
    Shader shader;
    Light lights[MAX_LIGHTS] = { 0 };
    explicit LightSubSystem();
    void LinkRenderableToLight(Renderable* renderable);
    void DrawDebugLights();
};
}
