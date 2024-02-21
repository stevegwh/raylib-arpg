//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once
#include <vector>

#include "Renderable.hpp"

namespace sage
{
    class RenderSystem
    {
        std::vector<std::unique_ptr<Renderable>> renderables;
    public:
        ~RenderSystem();
        void AddRenderable(Renderable& renderable);
        void Draw() const;
    };
}

