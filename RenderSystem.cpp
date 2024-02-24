//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"
#include "raylib.h"
#include "Game.hpp"

namespace sage
{
    RenderSystem::~RenderSystem()
    {
    
    }
    
    void RenderSystem::Draw() const
    {
        for (const auto& renderable : components)
        {
            const Transform* transform = sage::Game::GetInstance().transformSystem->GetComponent(renderable.second->entityId);
            DrawModel(renderable.second->model, transform->position, transform->scale, WHITE);
        }
        
    }
}

