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

    void RenderSystem::DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, std::string>& data)
    {
        int id = std::stoi(entityId);
    
        // Load material
        sage::Material mat = { .diffuse = LoadTexture(data.at("Material").c_str()), .path = data.at("Material") };
    
        // Load model
        std::string modelPath = data.at("Model");
        Model model = LoadModel(modelPath.c_str());
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
    
        // Create Renderable component and add to system
        auto renderable = std::make_unique<Renderable>(id, model, mat, modelPath);
        //components.emplace(id, std::move(renderable));
        AddComponent(std::move(renderable));
    }
}

