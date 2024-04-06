//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"
#include "raylib.h"
#include "GameManager.hpp"

namespace sage
{
RenderSystem::~RenderSystem()
{
    

}

void RenderSystem::Update()
{
    for (const auto& renderable : components)
    {
        if (renderable.second->anim && GetFrameTime() > 1)
        {
            renderable.second->animIndex = (renderable.second->animIndex + 1)%renderable.second->animsCount;
            // Update model animation
            ModelAnimation anim = renderable.second->animation[renderable.second->animIndex];
            renderable.second->animCurrentFrame = (renderable.second->animCurrentFrame + 1) % anim.frameCount;
            UpdateModelAnimation(renderable.second->model, anim, renderable.second->animCurrentFrame);
        }
    }
}

void RenderSystem::Draw() const
{
    for (const auto& renderable : components)
    {
        if (renderable.second->anim)
        {
            // Update model animation
            ModelAnimation anim = renderable.second->animation[renderable.second->animIndex];
            renderable.second->animCurrentFrame = (renderable.second->animCurrentFrame + 1) % anim.frameCount;
            UpdateModelAnimation(renderable.second->model, anim, renderable.second->animCurrentFrame);

            renderable.second->model.transform = renderable.second->initialTransform;
        }
        
        DrawModel(renderable.second->model, renderable.second->transform->position, renderable.second->transform->scale, WHITE);
    }
}

void RenderSystem::DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, 
                                         std::string>& data)
{
    int id = std::stoi(entityId);
    
    sage::Material mat = { .diffuse = LoadTexture(data.at("Material").c_str()), .path = data.at("Material") };
    
    std::string modelPath = data.at("Model");
    Model model = LoadModel(modelPath.c_str());
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
    
    auto renderable = std::make_unique<Renderable>(id, model, mat, modelPath, MatrixIdentity(), ECS->transformSystem->GetComponent(id));
    auto t = ECS->transformSystem->GetComponent(id);
    AddComponent(std::move(renderable));
}

}

