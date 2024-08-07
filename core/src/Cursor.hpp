//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "components/Collideable.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    enum class CursorState
    {
        DEFAULT,
        NPC_HOVER,
        BUILDING_HOVER
    };

    class Cursor
    {
        entt::registry* registry;
        GameData* gameData;

        CollisionInfo m_mouseHitInfo{};
        CollisionInfo m_terrainHitInfo{};

        Texture2D* currentTex;
        Texture2D regulartex{};
        Texture2D talktex{};
        Texture2D movetex{};
        Texture2D invalidmovetex{};
        Texture2D combattex{};

        Vector2 position{};
        Ray ray{};
        Color defaultColor = WHITE;
        Color hoverColor = LIME;
        Color invalidColor = RED;
        Color currentColor = WHITE;

        bool contextLocked = false;
        bool hideCursor = false;
        bool enabled = true;

        entt::entity controlledActor;

        void getMouseRayCollision();
        void onMouseClick();
        void changeCursors(CollisionLayer collisionLayer);
        static void resetHitInfo(CollisionInfo& hitInfo);
        void findMeshCollision(CollisionInfo& hitInfo);

      public:
        std::string hitObjectName{};
        [[nodiscard]] const CollisionInfo& getMouseHitInfo() const;
        [[nodiscard]] const RayCollision& terrainCollision() const;
        [[nodiscard]] const RayCollision& collision() const;

        entt::sigh<void(entt::entity)>
            onCollisionHit{}; // Returns the hit entity (all layers)
        entt::sigh<void(entt::entity)> onNPCClick{};
        entt::sigh<void(entt::entity entity)> onFloorClick{};
        entt::sigh<void(entt::entity entity)> onAnyClick{};
        entt::sigh<void(entt::entity)> onEnemyClick{};

        Cursor(entt::registry* registry, GameData* _gameData);

        void Update();
        void Draw3D();
        void Draw2D();
        void OnControlledActorChange(entt::entity entity);
        void DisableContextSwitching();
        void EnableContextSwitching();
        void Enable();
        void Disable();
        void Hide();
        void Show();
        bool isValidMove() const;
    };
} // namespace sage
