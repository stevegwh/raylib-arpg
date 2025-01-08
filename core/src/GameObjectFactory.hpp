//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class Scene;
    class GameData;

    struct GameObjectFactory
    {
        static void makeInteractable(entt::registry* registry, entt::entity entity);
        static entt::entity createDialogCutscene(entt::registry* registry, Vector3 position, const char* name);
        static entt::entity createEnemy(
            entt::registry* registry, GameData* data, Vector3 position, Vector3 rotation, const char* name);
        static entt::entity createCellGuard(
            entt::registry* registry, GameData* data, Vector3 position, const char* name);
        static entt::entity createLeverGoblin(
            entt::registry* registry, GameData* data, Vector3 position, const char* name);
        static entt::entity createArissa(
            entt::registry* registry, GameData* data, Vector3 position, Vector3 rotation);
        static entt::entity createPlayer(
            entt::registry* registry, GameData* data, Vector3 position, Vector3 rotation, const char* name);
        static void createPortal(entt::registry* registry, GameData* data, Vector3 position);
        static void createWizardTower(entt::registry* registry, GameData* data, Vector3 position);
        static bool spawnItemInWorld(
            entt::registry* registry, GameData* data, entt::entity itemId, Vector3 position);
    };
} // namespace sage
