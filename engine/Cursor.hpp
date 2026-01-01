//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "engine/components/Collideable.hpp"
#include "engine/Event.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <optional>

namespace sage
{
    class EngineSystems;

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
        EngineSystems* sys;

        entt::entity selectedActor = entt::null;

        sage::CollisionInfo m_mouseHitInfo{};
        sage::CollisionInfo m_naviHitInfo{};
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
        void changeCursors(sage::CollisionLayer collisionLayer);
        static void resetHitInfo(sage::CollisionInfo& hitInfo);
        [[nodiscard]] bool findMeshCollision(sage::CollisionInfo& hitInfo) const;

      public:
        std::string hitObjectName{};
        [[nodiscard]] const sage::CollisionInfo& getMouseHitInfo() const;
        [[nodiscard]] const RayCollision& getFirstNaviCollision() const;
        [[nodiscard]] const RayCollision& getFirstCollision() const;
        [[nodiscard]] entt::entity GetSelectedActor() const;
        void SetSelectedActor(entt::entity actor);

        sage::Event<entt::entity, entt::entity> onSelectedActorChange{}; // prev, current
        sage::Event<entt::entity> onCollisionHit{};                      // Returns the hit entity (all layers)
        sage::Event<entt::entity> onNPCClick{};
        sage::Event<entt::entity> onInteractableClick{};
        sage::Event<entt::entity> onItemClick{};
        sage::Event<entt::entity> onFloorClick{};
        sage::Event<entt::entity> onAnyLeftClick{};
        sage::Event<entt::entity> onAnyRightClick{};
        sage::Event<entt::entity> onEnemyLeftClick{};
        sage::Event<entt::entity> onEnemyRightClick{};
        sage::Event<entt::entity> onChestClick{};

        sage::Event<entt::entity> onCombatableHover{};
        sage::Event<entt::entity> onNPCHover{};
        sage::Event<entt::entity> onItemHover{};
        sage::Event<> onStopHover{};

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
        [[nodiscard]] bool OutOfRange() const;
        [[nodiscard]] bool IsValidMove() const;

        Cursor(entt::registry* registry, EngineSystems* _sys);
    };
} // namespace sage
