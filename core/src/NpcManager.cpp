//
// Created by Steve Wheeler on 02/12/2024.
//

#include "NpcManager.hpp"

#include "components/DialogComponent.hpp"
#include "components/Renderable.hpp"
#include "GameObjectFactory.hpp"

namespace sage
{

    // TODO: Currently this confuses 'NPC' with 'Speakable'. In reality, you could likely just use
    // FindRenderableByName for this, the only issue comes from name collisions.

    entt::entity NPCManager::CreateNPC(const std::string& name, Vector3 pos, Vector3 rot)
    {
        if (name == "Arissa")
        {
            return GameObjectFactory::createArissa(registry, gameData, pos, rot);
        }
        else if (name == "Cell_Guard")
        {
            return GameObjectFactory::createCellGuard(registry, gameData, pos, name.c_str());
        }
        else if (name == "Lever_Goblin")
        {
            return GameObjectFactory::createLeverGoblin(registry, gameData, pos, name.c_str());
        }

        return entt::null;
    }

    NPCManager::NPCManager(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

} // namespace sage