//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
#include "raylib.h"
#include <string>

#include "Material.hpp"
#include "Component.hpp"


namespace sage
{
    struct Renderable : public sage::Component
    {
        sage::Material material;
        Model model;
        std::string name = "Default";
        
        Renderable(EntityID entityId, Model _model, sage::Material _material)
        : Component(entityId), model(_model), material(_material)
        {
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = material.diffuse;
            // TODO: Need to update bounding box based on model position
        }
        
        ~Renderable()
        {
            UnloadModel(model);
            UnloadTexture(material.diffuse);
        }

    };
}

