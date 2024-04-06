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
        if (renderable.second->anim)
        {
            renderable.second->animationController->Update();
//            ModelAnimation anim = renderable.second->animation[renderable.second->animIndex];
//            renderable.second->animCurrentFrame = (renderable.second->animCurrentFrame + 1) % anim.frameCount;
//            UpdateModelAnimation(renderable.second->model, anim, renderable.second->animCurrentFrame);
        }
    }
}
//float rotationAngle = 0;
void RenderSystem::Draw() const
{
    //rotationAngle += 10.0f * GetFrameTime();
    //std::fmod(rotationAngle, 360);
    for (const auto& renderable : components)
    {
        auto t = renderable.second->transform;
        Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
        DrawModelEx(renderable.second->model, t->position, rotationAxis, t->rotation.y, {t->scale, t->scale, t->scale}, WHITE);
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

