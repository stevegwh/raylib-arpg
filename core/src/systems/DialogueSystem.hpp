//
// Created by steve on 11/05/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "Cursor.hpp"
#include "UserInput.hpp"
#include "../components/Dialogue.hpp"

#include <entt/entt.hpp>

namespace sage
{

class DialogueSystem : public BaseSystem<Dialogue>
{
    entt::entity hoveredEntity{};
    void onNPCClicked(entt::entity entity);
public:
    explicit DialogueSystem(entt::registry* registry, Cursor* _cursor, UserInput* _userInput);
};

} // sage

