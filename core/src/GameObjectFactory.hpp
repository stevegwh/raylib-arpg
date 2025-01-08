//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class Scene;
    class Systems;

    struct GameObjectFactory
    {
        static entt::entity createDialogCutscene(entt::registry* registry, Vector3 position, const char* name);
        static entt::entity createEnemy(
            entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation, const char* name);
        static entt::entity createCellGuard(
            entt::registry* registry, Systems* sys, Vector3 position, const char* name);
        static entt::entity createLeverGoblin(
            entt::registry* registry, Systems* sys, Vector3 position, const char* name);
        static entt::entity createArissa(
            entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation);
        static entt::entity createPlayer(
            entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation, const char* name);
        static void createPortal(entt::registry* registry, Systems* sys, Vector3 position);
        static void createWizardTower(entt::registry* registry, Systems* sys, Vector3 position);
        static bool spawnItemInWorld(
            entt::registry* registry, Systems* sys, entt::entity itemId, Vector3 position);
    };
} // namespace sage
