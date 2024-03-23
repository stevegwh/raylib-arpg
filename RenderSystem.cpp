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
            DrawModel(renderable.second->model, transform->position, 1.0f, WHITE);
        }
    }
    
    void RenderSystem::DeserializeComponents(const std::vector<std::unordered_map<std::string, std::string>>& data)
    {
        for (const auto& c: data)
        {
            sage::Material mat = { .diffuse = LoadTexture(c.at("Material").c_str()), .path = c.at("Material") };
            std::string modelPath = c.at("Model");
            int id = std::stoi(c.at("EntityId"));
            Model model = LoadModel(modelPath.c_str());
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
            auto renderable = std::make_unique<Renderable>(id, model, mat, modelPath);
            //AddComponent(std::move(renderable));
            components.emplace(id, std::move(renderable));
        }
    }
}

