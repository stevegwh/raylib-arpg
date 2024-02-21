//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"
#include "raylib.h"

namespace sage
{
    RenderSystem::~RenderSystem()
    {
    
    }
    
    void RenderSystem::AddRenderable(Renderable& renderable)
    {
        renderables.push_back(std::make_unique<Renderable>(renderable));
    }
    
    void RenderSystem::Draw() const
    {
        for (const auto& renderable : renderables)
        {
            //Transform transform = app->transformSystem->GetComponent(renderable->entityId);
            //DrawModel(renderable->model, transform.position, transform.scale, WHITE);
            DrawModel(renderable->model, {0, 0, 0}, 1.0f, WHITE);
        }
    }


}

