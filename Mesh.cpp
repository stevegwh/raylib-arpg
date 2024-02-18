//
// Created by Steve Wheeler on 18/02/2024.
//

#include "Mesh.hpp"


namespace sage
{
    void Mesh::Draw()
    {
        DrawModel(model, position, scale, WHITE);
    }
    
    void Mesh::SetScale(float _scale)
    {
        scale = _scale;
    }
    
    void Mesh::SetPosition(Vector3 _position)
    {
        position = _position;
    }
    
    void Mesh::SetBoundingBox(BoundingBox _bb)
    {
        boundingBox = _bb;
    }
    
    Mesh::~Mesh()
    {
        UnloadModel(model);
        UnloadTexture(material.diffuse);
    }
}