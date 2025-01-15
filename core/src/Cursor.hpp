//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "components/Collideable.hpp"
#include "Event.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <optional>

namespace sage
{
    class Systems;

    struct HoverInfo
    {
        entt::entity target = entt::null;
        double beginHoverTime = 0.0;
        const float hoverTimeThreshold = 0.75f;
    };

    class Cursor
    {
        float leftClickTimer = 0;
        entt::registry* registry;
        Systems* sys;

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
        Texture2D interacttex{};

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

        Event<entt::entity> onCollisionHit{}; // Returns the hit entity (all layers)
        Event<entt::entity> onNPCClick{};
        Event<entt::entity> onInteractableClick{};
        Event<entt::entity> onItemClick{};
        Event<entt::entity> onFloorClick{};
        Event<entt::entity> onAnyLeftClick{};
        Event<entt::entity> onAnyRightClick{};
        Event<entt::entity> onEnemyLeftClick{};
        Event<entt::entity> onEnemyRightClick{};

        Event<entt::entity> onCombatableHover{};
        Event<entt::entity> onNPCHover{};
        Event<entt::entity> onItemHover{};
        Event<> onStopHover{};

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
        [[nodiscard]] bool IsValidMove() const;

        Cursor(entt::registry* registry, Systems* _sys);
    };
} // namespace sage
