//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "Renderable.hpp"
#include "Transform.hpp"
#include "BaseSystem.hpp"

#include <unordered_map>
#include <string>

namespace sage
{
class RenderSystem : public BaseSystem<Renderable>
{
public:
    ~RenderSystem();
    void Update();
    void Draw() const;
    void DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, std::string>& data);
};
}

