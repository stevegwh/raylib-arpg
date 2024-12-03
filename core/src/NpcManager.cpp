//
// Created by Steve Wheeler on 02/12/2024.
//

#include "NpcManager.hpp"

#include "components/DialogComponent.hpp"
#include "components/Renderable.hpp"

namespace sage
{

    entt::entity NPCManager::GetNPC(const std::string& name)
    {
        return npcMap[name];
    }

    void NPCManager::onComponentAdded(entt::entity entity)
    {
        const auto& renderable = registry->get<Renderable>(entity);
        npcMap.emplace(renderable.name, entity);
    }

    void NPCManager::onComponentRemoved(entt::entity entity)
    {
    }

    NPCManager::NPCManager(entt::registry* _registry) : registry(_registry)
    {
        // TODO: What if we want an NPC that doesn't speak?
        registry->on_construct<DialogComponent>().connect<&NPCManager::onComponentAdded>(this);
        registry->on_destroy<DialogComponent>().connect<&NPCManager::onComponentRemoved>(this);
    }

} // namespace sage