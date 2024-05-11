//
// Created by steve on 11/05/2024.
//

#include "DialogueSystem.hpp"
#include "components/Animation.hpp"

#include <iostream>

namespace sage
{
void DialogueSystem::onNPCClicked(entt::entity clickedNPC)
{
    // TODO:
    // Check distance from player
    // If above threshold, pathfind to location
    // Hook into onFinishMovement event of controlled actor transform
    // On finish movement, trigger below
    std::cout << registry->get<Dialogue>(clickedNPC).sentence << std::endl;
    registry->get<Animation>(clickedNPC).ChangeAnimation(1); // TODO: Change to an enum
}

DialogueSystem::DialogueSystem(entt::registry *registry, Cursor* _cursor) : BaseSystem(registry)
{
    {
        entt::sink sink{_cursor->onNPCClick};
        sink.connect<&DialogueSystem::onNPCClicked>(this);
    }
}
} // sage