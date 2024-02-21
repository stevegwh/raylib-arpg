//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
#include <raylib.h>
#include "Material.hpp"
#include "Transform.hpp"
#include <string>
namespace sage
{
    class Renderable
    {
        sage::Material material;
    public:
        const sage::Transform* const transform;
        std::string name = "Default";
        Renderable(Model _model, sage::Material _material, Transform* _transform)
        : model(_model), material(_material), transform(_transform)
        {
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = material.diffuse;
            // TODO: Need to update bounding box based on model position
        }
        ~Renderable();
        void Draw() const;
        Model model;
    };
}

