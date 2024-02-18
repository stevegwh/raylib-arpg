//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
#include <raylib.h>
#include "Material.hpp"
namespace sage
{
    class Mesh
    {


        sage::Material material;
        Vector3 position{};
        float scale{};
    public:
        Mesh(Model _model, sage::Material _material)
        : model(_model), material(_material)
        {
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = material.diffuse;
            boundingBox = GetMeshBoundingBox(model.meshes[0]);
        }
        ~Mesh();
        void Draw();
        void SetScale(float _scale);
        void SetPosition(Vector3 _position);
        void SetBoundingBox(BoundingBox _bb);
        Model model;
        BoundingBox boundingBox{};
    };
}

