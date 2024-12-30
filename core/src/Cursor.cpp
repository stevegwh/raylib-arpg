//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Cursor.hpp"

#include "GameData.hpp"

#include "Camera.hpp"
#include "components/CombatableActor.hpp"
#include "components/ItemComponent.hpp"
#include "components/MoveableActor.hpp"
#include "components/NavigationGridSquare.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "GameUiFactory.hpp"
#include "Settings.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include <algorithm>

#ifndef FLT_MAX
#define FLT_MAX                                                                                                   \
    340282346638528859811704183484516925440.0f // Maximum value of a float, from bit
                                               // pattern 01111111011111111111111111111111
#endif

namespace sage
{
    void Cursor::checkMouseHover()
    {
        if (!registry->any_of<Collideable>(m_mouseHitInfo.collidedEntityId)) return;
        const auto& layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        if ((layer != CollisionLayer::NPC && layer != CollisionLayer::ENEMY && layer != CollisionLayer::ITEM) ||
            !m_mouseHitInfo.rlCollision.hit)
        {
            if (m_hoverInfo.has_value())
            {
                onStopHover.publish();
            }
            m_hoverInfo.reset();
            return;
        }
        if (!m_hoverInfo.has_value() || m_mouseHitInfo.collidedEntityId != m_hoverInfo->target)
        {
            HoverInfo newInfo;
            newInfo.target = m_mouseHitInfo.collidedEntityId;
            newInfo.beginHoverTime = GetTime();
            m_hoverInfo.emplace(newInfo);
        }
    }

    void Cursor::onMouseHover() const
    {
        if (!enabled) return;

        const auto& layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        if (layer == CollisionLayer::NPC || layer == CollisionLayer::INTERACTABLE)
        {
            onNPCHover.publish(m_mouseHitInfo.collidedEntityId);
        }
        else if (layer == CollisionLayer::ITEM)
        {
            onItemHover.publish(m_mouseHitInfo.collidedEntityId);
        }
        else if (layer == CollisionLayer::ENEMY || layer == CollisionLayer::PLAYER)
        {
            onCombatableHover.publish(m_mouseHitInfo.collidedEntityId);
        }
    }

    void Cursor::onMouseLeftClick() const
    {
        if (!enabled) return;

        const auto& layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        if (layer == CollisionLayer::NPC)
        {
            onNPCClick.publish(m_mouseHitInfo.collidedEntityId);
        }
        else if (layer == CollisionLayer::ITEM)
        {
            onItemClick.publish(m_mouseHitInfo.collidedEntityId);
        }
        else if (layer == CollisionLayer::INTERACTABLE)
        {
            // Could call it 'inspectable' instead?
            // Interactables are essentially just NPCs but without an actor name/portrait
            onNPCClick.publish(m_mouseHitInfo.collidedEntityId);
            // onInteractableClick.publish(m_mouseHitInfo.collidedEntityId);
        }
        else if (
            layer == CollisionLayer::FLOORSIMPLE || layer == CollisionLayer::FLOORCOMPLEX ||
            layer == CollisionLayer::STAIRS)
        {
            onFloorClick.publish(m_mouseHitInfo.collidedEntityId);
        }
        else if (layer == CollisionLayer::ENEMY)
        {
            onEnemyLeftClick.publish(m_mouseHitInfo.collidedEntityId);
        }
        onAnyLeftClick.publish(m_mouseHitInfo.collidedEntityId);
    }

    void Cursor::onMouseRightClick() const
    {
        if (!enabled) return;

        const auto& layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        if (layer == CollisionLayer::ENEMY)
        {
            onEnemyRightClick.publish(m_mouseHitInfo.collidedEntityId);
        }
        onAnyRightClick.publish(m_mouseHitInfo.collidedEntityId);
    }

    void Cursor::onMouseLeftDown()
    {
        if (!enabled) return;
        leftClickTimer += GetFrameTime();

        if (leftClickTimer < 0.25) return;
        leftClickTimer = 0;

        const auto& layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        if (layer == CollisionLayer::FLOORSIMPLE || layer == CollisionLayer::FLOORCOMPLEX ||
            layer == CollisionLayer::STAIRS)
        {
            onFloorClick.publish(m_mouseHitInfo.collidedEntityId);
        }
        else if (layer == CollisionLayer::ENEMY)
        {
            onEnemyLeftClick.publish(m_mouseHitInfo.collidedEntityId);
        }
    }

    void Cursor::onMouseRightDown() const
    {
        if (!enabled) return;
    }

    void Cursor::DisableContextSwitching() // Lock mouse context? Like changing depending
                                           // on getFirstCollision.
    {
        contextLocked = true;
    }

    void Cursor::EnableContextSwitching()
    {
        contextLocked = false;
    }

    void Cursor::Enable()
    {
        enabled = true;
    }

    void Cursor::Disable()
    {
        enabled = false;
    }

    void Cursor::Hide()
    {
        hideCursor = true;
    }

    void Cursor::Show()
    {
        hideCursor = false;
    }

    bool Cursor::isValidMove() const
    {
        auto mouseHit = m_naviHitInfo.rlCollision.point;
        if (gameData->navigationGridSystem->CheckWithinGridBounds(mouseHit))
        {
            const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
            const auto& actor = registry->get<MoveableActor>(selectedActor);
            GridSquare minRange{};
            GridSquare maxRange{};
            gameData->navigationGridSystem->GetPathfindRange(
                selectedActor, actor.pathfindingBounds, minRange, maxRange);

            if (!gameData->navigationGridSystem->CheckWithinBounds(mouseHit, minRange, maxRange))
            {
                // Out of player's movement range
                return false;
            }
        }
        else
        {
            return false;
        }
        GridSquare dest{};
        gameData->navigationGridSystem->WorldToGridSpace(mouseHit, dest);
        if (gameData->navigationGridSystem->GetGridSquare(dest.row, dest.col)->occupied)
        {
            return false;
        }
        return true;
    }

    void Cursor::changeCursors(CollisionLayer layer)
    {
        if (contextLocked) return;

        if (layer == CollisionLayer::FLOORSIMPLE || layer == CollisionLayer::FLOORCOMPLEX ||
            layer == CollisionLayer::STAIRS)
        {
            if (isValidMove())
            {
                currentTex = &movetex;
                currentColor = GREEN;
            }
            else
            {
                currentTex = &invalidmovetex;
                currentColor = invalidColor;
            }
            if (registry->all_of<Renderable>(m_mouseHitInfo.collidedEntityId))
            {
                hitObjectName = registry->get<Renderable>(m_mouseHitInfo.collidedEntityId).name;
            }
        }
        else if (layer == CollisionLayer::BUILDING)
        {
            currentTex = &regulartex;
            currentColor = invalidColor;
            if (registry->all_of<Renderable>(m_mouseHitInfo.collidedEntityId))
            {
                hitObjectName = registry->get<Renderable>(m_mouseHitInfo.collidedEntityId).name;
            }
        }
        else if (layer == CollisionLayer::ITEM)
        {
            currentTex = &pickuptex;
            hitObjectName = "Item";
        }
        else if (layer == CollisionLayer::PLAYER)
        {
            currentTex = &regulartex;
            hitObjectName = "Player";
        }
        else if (layer == CollisionLayer::NPC)
        {
            currentTex = &talktex;
            hitObjectName = "NPC";
        }
        else if (layer == CollisionLayer::INTERACTABLE)
        {
            currentTex = &interacttex;
            hitObjectName = "Interactable";
        }
        else if (layer == CollisionLayer::ENEMY)
        {
            currentTex = &combattex;
            hitObjectName = "Enemy";
        }
    }

    void Cursor::getMouseRayCollision()
    {
        // Reset hit information
        resetHitInfo(m_mouseHitInfo);
        resetHitInfo(m_naviHitInfo);
        hitObjectName = "None";
        currentTex = &regulartex;
        currentColor = defaultColor;

        auto viewport = gameData->settings->GetViewPort();
        // Get ray and test against objects
        ray = GetScreenToWorldRayEx(GetMousePosition(), *gameData->camera->getRaylibCam(), viewport.x, viewport.y);
        auto collisions = gameData->collisionSystem->GetCollisionsWithRay(ray);

        // Replace floor BB hit with mesh hit then re-sort vector
        // Discards hits with a BB that do not have a collision with mesh
        for (auto it = collisions.begin(); it != collisions.end();)
        {
            if (it->collisionLayer == CollisionLayer::FLOORCOMPLEX || it->collisionLayer == CollisionLayer::STAIRS)
            {
                if (!findMeshCollision(*it))
                {
                    it = collisions.erase(it);
                    continue;
                }
            }

            ++it;
        }

        if (collisions.empty()) // Could put this sooner, but would need to repeat after above
        {
            return;
        }

        CollisionSystem::SortCollisionsByDistance(collisions);

        m_mouseHitInfo = collisions[0];

        if (m_mouseHitInfo.collisionLayer == CollisionLayer::FLOORSIMPLE ||
            m_mouseHitInfo.collisionLayer == CollisionLayer::FLOORCOMPLEX ||
            m_mouseHitInfo.collisionLayer == CollisionLayer::STAIRS)
        {
            m_naviHitInfo = m_mouseHitInfo;
        }
        else
        {
            // Find first navigation collision (if any)
            const auto navIt = std::find_if(collisions.begin(), collisions.end(), [](const CollisionInfo& coll) {
                return coll.collisionLayer == CollisionLayer::FLOORSIMPLE ||
                       coll.collisionLayer == CollisionLayer::FLOORCOMPLEX ||
                       coll.collisionLayer == CollisionLayer::STAIRS;
            });

            if (navIt != collisions.end())
            {
                m_naviHitInfo = *navIt;
            }
        }

        onCollisionHit.publish(m_mouseHitInfo.collidedEntityId);

        const auto layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        changeCursors(layer);
    }

    void Cursor::resetHitInfo(CollisionInfo& hitInfo)
    {
        hitInfo.rlCollision = {};
        hitInfo.rlCollision.distance = FLT_MAX;
        hitInfo.rlCollision.hit = false;
    }

    // Find the model's mesh collision (instead of using its bounding box)
    bool Cursor::findMeshCollision(CollisionInfo& hitInfo) const
    {
        if (registry->any_of<Renderable>(hitInfo.collidedEntityId))
        {
            const auto& renderable = registry->get<Renderable>(hitInfo.collidedEntityId);
            const auto& transform = registry->get<sgTransform>(hitInfo.collidedEntityId);

            for (int i = 0; i < renderable.GetModel()->GetMeshCount(); ++i)
            {
                if (const auto meshCollision =
                        renderable.GetModel()->GetRayMeshCollision(ray, i, transform.GetMatrix());
                    meshCollision.hit)
                {
                    hitInfo.rlCollision = meshCollision;
                    return true;
                }
            }
        }
        return false;
    }

    const CollisionInfo& Cursor::getMouseHitInfo() const
    {
        return m_mouseHitInfo;
    }

    const RayCollision& Cursor::getFirstNaviCollision() const
    {
        return m_naviHitInfo.rlCollision;
    }

    const RayCollision& Cursor::getFirstCollision() const
    {
        return m_mouseHitInfo.rlCollision;
    }

    void Cursor::Update()
    {
        getMouseRayCollision();
        checkMouseHover();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            onMouseLeftClick();
            leftClickTimer = 0;
        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            onMouseRightClick();
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            onMouseLeftDown();
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            onMouseRightDown();
        }
        else if (
            m_hoverInfo.has_value() &&
            GetTime() >= m_hoverInfo.value().beginHoverTime + m_hoverInfo.value().hoverTimeThreshold)
        {
            onMouseHover();
        }
    }

    void Cursor::DrawDebug() const
    {
        if (!m_mouseHitInfo.rlCollision.hit) return;
        if (contextLocked) return;
        DrawCube(m_mouseHitInfo.rlCollision.point, 0.5f, 0.5f, 0.5f, currentColor);
        Vector3 normalEnd;
        normalEnd.x = m_mouseHitInfo.rlCollision.point.x + m_mouseHitInfo.rlCollision.normal.x;
        normalEnd.y = m_mouseHitInfo.rlCollision.point.y + m_mouseHitInfo.rlCollision.normal.y;
        normalEnd.z = m_mouseHitInfo.rlCollision.point.z + m_mouseHitInfo.rlCollision.normal.z;
        DrawLine3D(m_mouseHitInfo.rlCollision.point, normalEnd, RED);
    }

    void Cursor::Draw3D()
    {
    }

    void Cursor::Draw2D() const
    {
        if (hideCursor) return;
        Vector2 pos = GetMousePosition();
        if (currentTex != &regulartex)
        {
            pos = Vector2Subtract(
                pos, {static_cast<float>(currentTex->width / 2), static_cast<float>(currentTex->height / 2)});
        }
        DrawTextureEx(*currentTex, pos, 0.0, 1.0f, WHITE);
    }

    Cursor::Cursor(entt::registry* _registry, GameData* _gameData) : registry(_registry), gameData(_gameData)
    {
        regulartex = ResourceManager::GetInstance().TextureLoad("IMG_CURSOR_REGULAR");
        talktex = ResourceManager::GetInstance().TextureLoad("IMG_CURSOR_TALK");
        movetex = ResourceManager::GetInstance().TextureLoad("IMG_CURSOR_MOVE");
        invalidmovetex = ResourceManager::GetInstance().TextureLoad("IMG_CURSOR_DENIED");
        combattex = ResourceManager::GetInstance().TextureLoad("IMG_CURSOR_ATTACK");
        pickuptex = ResourceManager::GetInstance().TextureLoad("IMG_CURSOR_PICKUP");
        interacttex = ResourceManager::GetInstance().TextureLoad("IMG_CURSOR_INTERACT");
        currentTex = &regulartex;
        EnableContextSwitching();
    }
} // namespace sage
