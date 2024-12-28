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
        static entt::entity createEmissiveCrystal(
            entt::registry* registry, GameData* data, Vector3 position, const char* name);
        static entt::entity createEnemy(
            entt::registry* registry, GameData* data, Vector3 position, Vector3 rotation, const char* name);
        static entt::entity createQuestNPC(
            entt::registry* registry, GameData* data, Vector3 position, const char* name);
        static entt::entity createFetchQuestNPC(
            entt::registry* registry, GameData* data, Vector3 position, const char* name);
        static entt::entity createKnight(
            entt::registry* registry, GameData* data, Vector3 position, Vector3 rotation, const char* name);
        static entt::entity createPlayer(
            entt::registry* registry, GameData* data, Vector3 position, Vector3 rotation, const char* name);
        static void createPortal(entt::registry* registry, GameData* data, Vector3 position);
        static void createWizardTower(entt::registry* registry, GameData* data, Vector3 position);
        static bool spawnItemInWorld(
            entt::registry* registry, GameData* data, entt::entity itemId, Vector3 position);
    };
} // namespace sage
