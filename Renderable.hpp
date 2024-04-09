//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
#include "raylib.h"
#include "raymath.h"

#include <string>
#include <iostream>
#include <utility>
#include <vector>
#include <optional>

#include "Material.hpp"
#include "Component.hpp"
#include "RenderSystem.hpp"
#include "Transform.hpp"

namespace sage
{
struct Renderable : public Component<Renderable>
{
    const sage::Transform* const transform; // Hard dependency
    const Matrix initialTransform;
    sage::Material material;
    const std::string modelPath;
    Model model; // was const
    std::optional<Shader> shader;

    std::string name = "Default";
    
    Renderable(
        EntityID entityId, 
        Model _model, 
        sage::Material _material, 
        std::string _modelPath, 
        Matrix _localTransform,
        const Transform* const _transform)
    : 
    Component(entityId),
    model(std::move(_model)), 
    material(std::move(_material)), 
    modelPath(_modelPath), 
    initialTransform(_localTransform),
    transform(_transform)
    {
        model.transform = initialTransform;
    }
    
    Renderable(
        EntityID entityId, 
        Model _model, 
        std::string _modelPath,
        Matrix _localTransform,
        const Transform* const _transform)
        : 
        Component(entityId), 
        model(std::move(_model)), 
        modelPath(_modelPath), 
        initialTransform(_localTransform),
        transform(_transform)
    {
        model.transform = initialTransform;
    }
    
    ~Renderable()
    {
        if (shader.has_value())
        {
            UnloadShader(shader.value());
        }
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

    [[nodiscard]] BoundingBox CalculateModelBoundingBox() const
    {
        Mesh mesh = model.meshes[0];
        std::vector<float> vertices(mesh.vertexCount * 3);
        memcpy(&vertices[0], mesh.vertices, sizeof(float) * mesh.vertexCount * 3);

        BoundingBox bb;
        bb.min = {0, 0, 0};
        bb.max = {0, 0, 0};

        {
            float x = vertices[0];
            float y = vertices[1];
            float z = vertices[2];

            Vector3 v = {x, y, z};
            // Assuming rl.Vector3Transform is a function that transforms a Vector3
            // using the given transform.
            v = Vector3Transform(v, model.transform);

            bb.min = bb.max = v;
        }

        for (size_t i = 0; i < vertices.size(); i += 3) {
            float x = vertices[i];
            float y = vertices[i + 1];
            float z = vertices[i + 2];

            Vector3 v = {x, y, z};
            v = Vector3Transform(v, model.transform);

            bb.min.x = std::min(bb.min.x, v.x);
            bb.min.y = std::min(bb.min.y, v.y);
            bb.min.z = std::min(bb.min.z, v.z);

            bb.max.x = std::max(bb.max.x, v.x);
            bb.max.y = std::max(bb.max.y, v.y);
            bb.max.z = std::max(bb.max.z, v.z);
        }
        
        return bb;
    }

};
}

