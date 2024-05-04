//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"
#include "raylib.h"
#include "../GameManager.hpp"

namespace sage
{
RenderSystem::~RenderSystem()
{
}

void RenderSystem::Update()
{
}
//float rotationAngle = 0;
void RenderSystem::Draw() const
{
    const auto& view = registry->view<Renderable, Transform>();
    view.each([](const auto& r, const auto& t) {
        Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
        DrawModelEx(r.model, t.position, rotationAxis, t.rotation.y, {t.scale, t.scale, t.scale}, WHITE);
    });
}

void RenderSystem::DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, 
                                         std::string>& data)
{
    int id = std::stoi(entityId);
    
    sage::Material mat = { .diffuse = LoadTexture(data.at("Material").c_str()), .path = data.at("Material") };
    
    std::string modelPath = data.at("Model");
    Model model = LoadModel(modelPath.c_str());
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
    
    auto renderable = std::make_unique<Renderable>(model, mat, modelPath, MatrixIdentity());
    //auto t = ECS->transformSystem->GetComponent(id);
    //AddComponent(std::move(renderable));
}

RenderSystem::RenderSystem(entt::registry* _registry) : BaseSystem<Renderable>(_registry) {}

}

