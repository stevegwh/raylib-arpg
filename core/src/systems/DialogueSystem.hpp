//
// Created by steve on 11/05/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "../components/Dialogue.hpp"

namespace sage
{

class DialogueSystem : public BaseSystem<Dialogue>
{
public:
    explicit DialogueSystem(entt::registry* registry);
};

} // sage

