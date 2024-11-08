//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "components/Collideable.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

#include <optional>

namespace sage
{
    struct HoverInfo
    {
        entt::entity target = entt::null;
        double beginHoverTime = 0.0;
        const float hoverTimeThreshold = 0.25f;
    };

    class GameData;

    class Cursor
    {
        float leftClickTimer = 0;
        entt::registry* registry;
        GameData* gameData;

        CollisionInfo m_mouseHitInfo{};
        CollisionInfo m_naviHitInfo{};
        std::optional<HoverInfo> m_hoverInfo{};

        Texture2D* currentTex;
        Texture2D regulartex{};
        Texture2D talktex{};
        Texture2D movetex{};
        Texture2D invalidmovetex{};
        Texture2D combattex{};
        Texture2D pickuptex{};

        Ray ray{};
        Color defaultColor = WHITE;
        Color hoverColor = LIME;
        Color invalidColor = RED;
        Color currentColor = WHITE;

        bool contextLocked = false;
        bool hideCursor = false;
        bool enabled = true;

        void getMouseRayCollision();
        void checkMouseHover();
        void onMouseHover() const;
        void onMouseLeftClick() const;
        void onMouseRightClick() const;
        void onMouseLeftDown();
        void onMouseRightDown() const;
        void changeCursors(CollisionLayer collisionLayer);
        static void resetHitInfo(CollisionInfo& hitInfo);
        [[nodiscard]] bool findMeshCollision(CollisionInfo& hitInfo) const;

      public:
        std::string hitObjectName{};
        [[nodiscard]] const CollisionInfo& getMouseHitInfo() const;
        [[nodiscard]] const RayCollision& getFirstNaviCollision() const;
        [[nodiscard]] const RayCollision& getFirstCollision() const;

        entt::sigh<void(entt::entity)> onCollisionHit{}; // Returns the hit entity (all layers)
        entt::sigh<void(entt::entity)> onNPCClick{};
        entt::sigh<void(entt::entity entity)> onItemClick{};
        entt::sigh<void(entt::entity entity)> onFloorClick{};
        entt::sigh<void(entt::entity entity)> onAnyLeftClick{};
        entt::sigh<void(entt::entity entity)> onAnyRightClick{};
        entt::sigh<void(entt::entity)> onEnemyLeftClick{};
        entt::sigh<void(entt::entity)> onEnemyRightClick{};

        entt::sigh<void(entt::entity entity)> onCombatableHover{};
        entt::sigh<void(entt::entity entity)> onNPCHover{};
        entt::sigh<void(entt::entity entity)> onItemHover{};
        entt::sigh<void()> onStopHover{};

        void Update();
        void DrawDebug() const;
        void Draw3D();
        void Draw2D() const;
        void DisableContextSwitching();
        void EnableContextSwitching();
        void Enable();
        void Disable();
        void Hide();
        void Show();
        [[nodiscard]] bool isValidMove() const;

        Cursor(entt::registry* registry, GameData* _gameData);
    };
} // namespace sage
