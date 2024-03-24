//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
#include "raylib.h"
#include <string>
#include <iostream>
#include <utility>

#include "Material.hpp"
#include "Component.hpp"


namespace sage
{
    struct Renderable : public Component<Renderable>
    {
        sage::Material material;
        const std::string modelPath;
        const Model model;
        const BoundingBox meshBoundingBox;
        std::string name = "Default";
        
        Renderable(EntityID entityId, Model _model, sage::Material _material, std::string _modelPath)
        : Component(entityId), model(std::move(_model)), material(std::move(_material)), modelPath(_modelPath), meshBoundingBox(GetMeshBoundingBox(model.meshes[0]))
        {
        }
        
        ~Renderable()
        {
            UnloadModel(model);
            UnloadTexture(material.diffuse);
        }

        [[nodiscard]] std::unordered_map<std::string, std::string> SerializeImpl() const
        {
            return {
                {"EntityId", TextFormat("%i", entityId)},
                {"Material", TextFormat("%s", material.path.c_str())},
                {"Model", TextFormat("%s", modelPath.c_str())},
            };
        }

    };
}

