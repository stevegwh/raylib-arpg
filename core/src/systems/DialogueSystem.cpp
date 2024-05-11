//
// Created by steve on 11/05/2024.
//

#include "DialogueSystem.hpp"

#include <iostream>

namespace sage
{
void DialogueSystem::onNPCClicked(entt::entity entity)
{
    std::cout << "NPC clicked \n";
}

DialogueSystem::DialogueSystem(entt::registry *registry, Cursor* _cursor, UserInput* _userInput) : BaseSystem(registry)
{
    {
        entt::sink sink{_cursor->onNPCClick};
        sink.connect<&DialogueSystem::onNPCClicked>(this);
    }
}
} // sage