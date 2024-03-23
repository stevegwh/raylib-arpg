//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once
#include <map>

#include "Renderable.hpp"
#include "BaseSystem.hpp"

namespace sage
{
class RenderSystem : public BaseSystem<Renderable>
{
public:
    ~RenderSystem();
    void Draw() const;
    void DeserializeComponents(const std::vector<std::unordered_map<std::string, std::string>>& data);
    RenderSystem() : BaseSystem<Renderable>("Renderable") {}
};
}

