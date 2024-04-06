//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
#include "raylib.h"
#include "raymath.h"
#include <string>
#include <iostream>
#include <utility>

#include "Material.hpp"
#include "Component.hpp"


namespace sage
{
    struct Renderable : public Component<Renderable>
    {
        Vector3 position;
        float scale;
        Matrix initialTransform; // Make const
        
        sage::Material material;
        const std::string modelPath;
        Model model; // was const
        BoundingBox meshBoundingBox; // was const

        std::string name = "Default";

        bool anim = false;
        ModelAnimation* animation;
        unsigned int animIndex = 0;
        unsigned int animCurrentFrame = 0;
        int animsCount;
        
        Renderable(EntityID entityId, Model _model, sage::Material _material, std::string _modelPath)
        : Component(entityId), model(std::move(_model)), material(std::move(_material)), modelPath(_modelPath), meshBoundingBox(GetMeshBoundingBox(model.meshes[0]))
        {
        }

        // TODO: Currently defaults to having animations if no material is passed
        Renderable(EntityID entityId, Model _model, std::string _modelPath)
            : Component(entityId), model(std::move(_model)), modelPath(_modelPath), meshBoundingBox(GetMeshBoundingBox(model.meshes[0]))
        {
            animsCount = 0;
            animation = LoadModelAnimations(_modelPath.c_str(), &animsCount);
            initialTransform = MatrixMultiply(MatrixScale(0.035f, 0.035f, 0.035f) , MatrixRotateX(DEG2RAD*90));
            model.transform = initialTransform;
            GetMeshBoundingBox(model.meshes[0]);
        }
        
        ~Renderable()
        {
            UnloadModel(model);
            UnloadTexture(material.diffuse);
            UnloadModelAnimations(animation, animsCount);
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

