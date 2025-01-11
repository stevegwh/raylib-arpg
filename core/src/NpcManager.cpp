//
// Created by Steve Wheeler on 02/12/2024.
//

#include "NpcManager.hpp"

#include "components/DialogComponent.hpp"
#include "components/Renderable.hpp"
#include "GameObjectFactory.hpp"

namespace sage
{

    entt::entity NPCManager::CreateNPC(const std::string& name, Vector3 pos, Vector3 rot)
    {
        if (name == "Arissa")
        {
            return GameObjectFactory::createArissa(registry, sys, pos, rot);
        }
        else if (name.find("Goblin") != std::string::npos || name.find("Cell_Guard") != std::string::npos)
        {
            return GameObjectFactory::createGoblinNPC(registry, sys, pos, rot, name.c_str());
        }

        return entt::null;
    }

    NPCManager::NPCManager(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
    }

} // namespace sage