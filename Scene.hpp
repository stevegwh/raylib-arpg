//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

namespace sage
{

class Scene
{
public:
    virtual ~Scene() = default;
    virtual void Update() = 0;
    virtual void Draw3D() = 0;
    virtual void Draw2D() = 0;
};

} // sage
