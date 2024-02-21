//
// Created by Steve Wheeler on 18/02/2024.
//

#include "Renderable.hpp"


namespace sage
{
    void Renderable::Draw() const
    {
        DrawModel(model, transform->position, transform->scale, WHITE);
    }
    
    Renderable::~Renderable()
    {
        UnloadModel(model);
        UnloadTexture(material.diffuse);
    }
}