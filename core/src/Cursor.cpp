//
// Created by Steve Wheeler on 04/05/2024.
//

#include "Cursor.hpp"

#include "GameData.hpp"

#include "Camera.hpp"
#include "components/ControllableActor.hpp"
#include "components/NavigationGridSquare.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include <algorithm>

#ifndef FLT_MAX
#define FLT_MAX                                                                                                   \
    340282346638528859811704183484516925440.0f // Maximum value of a float, from bit
                                               // pattern 01111111011111111111111111111111
#endif

namespace sage
{
    void Cursor::onMouseLeftClick()
    {
        if (!enabled) return;

        const auto& layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        if (layer == CollisionLayer::NPC)
        {
            onNPCClick.publish(m_mouseHitInfo.collidedEntityId);
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

    void Cursor::onMouseRightClick()
    {
        if (!enabled) return;

        const auto& layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        if (layer == CollisionLayer::ENEMY)
        {
            onEnemyRightClick.publish(m_mouseHitInfo.collidedEntityId);
        }
        onAnyRightClick.publish(m_mouseHitInfo.collidedEntityId);
    }
    static float timer = 0;
    void Cursor::onMouseLeftDown()
    {
        if (!enabled) return;
        timer += GetFrameTime();

        if (timer < 0.5) return;
        timer = 0;

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

    void Cursor::onMouseRightDown()
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
        GridSquare clickedSquare{};
        if (gameData->navigationGridSystem->WorldToGridSpace(m_mouseHitInfo.rlCollision.point, clickedSquare))
        // Out of map bounds (TODO: Potentially pointless, if FLOORSIMPLE is the same size as
        // bounds.)
        {
            if (registry->any_of<ControllableActor>(controlledActor))
            {
                const auto& actor = registry->get<ControllableActor>(controlledActor);
                GridSquare minRange{};
                GridSquare maxRange{};
                gameData->navigationGridSystem->GetPathfindRange(
                    controlledActor, actor.pathfindingBounds, minRange, maxRange);
                if (!gameData->navigationGridSystem->WorldToGridSpace(
                        m_mouseHitInfo.rlCollision.point, clickedSquare, minRange, maxRange))
                // Out of player's movement range
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
        if (gameData->navigationGridSystem->GetGridSquare(clickedSquare.row, clickedSquare.col)->occupied)
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
        else if (layer == CollisionLayer::ENEMY)
        {
            currentTex = &combattex;
            hitObjectName = "NPC";
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

        // Get ray and test against objects
        ray = GetMouseRay(GetMousePosition(), *gameData->camera->getRaylibCam());
        auto collisions = gameData->collisionSystem->GetCollisionsWithRay(ray);

        // Replace floor BB hit with mesh hit then re-sort vector
        // Discards hits with a BB that do not have a collision with mesh
        for (auto it = collisions.begin(); it != collisions.end();)
        {
            if (it->collisionLayer == CollisionLayer::FLOORCOMPLEX)
            {
                if (!findMeshCollision(*it))
                {
                    it = collisions.erase(it);
                    continue;
                }
            }
            // TODO: Stairs

            ++it;
        }

        if (collisions.empty()) // Could put this sooner, but would need to repeat after above
        {
            return;
        }

        gameData->collisionSystem->SortCollisionsByDistance(collisions);

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
            auto navIt = std::find_if(collisions.begin(), collisions.end(), [](const CollisionInfo& coll) {
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

        auto layer = registry->get<Collideable>(m_mouseHitInfo.collidedEntityId).collisionLayer;
        changeCursors(layer);
    }

    void Cursor::resetHitInfo(CollisionInfo& hitInfo)
    {
        hitInfo.rlCollision = {};
        hitInfo.rlCollision.distance = FLT_MAX;
        hitInfo.rlCollision.hit = false;
    }

    // Find the model's mesh collision (instead of using its bounding box)
    bool Cursor::findMeshCollision(CollisionInfo& hitInfo)
    {
        if (registry->any_of<Renderable>(hitInfo.collidedEntityId))
        {
            auto& renderable = registry->get<Renderable>(hitInfo.collidedEntityId);
            auto& transform = registry->get<sgTransform>(hitInfo.collidedEntityId);

            for (int i = 0; i < renderable.GetModel()->GetMeshCount(); ++i)
            {
                auto meshCollision = renderable.GetModel()->GetRayMeshCollision(ray, i, transform.GetMatrix());
                if (meshCollision.hit)
                {
                    hitInfo.rlCollision = meshCollision;
                    return true;
                }
            }
        }
        return false;
    }

    void Cursor::OnControlledActorChange(entt::entity entity)
    {
        controlledActor = entity;
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
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            onMouseLeftClick();
            timer = 0;
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
    }

    void Cursor::DrawDebug()
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

    void Cursor::Draw2D()
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
        regulartex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_CURSOR_REGULAR);
        talktex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_CURSOR_TALK);
        movetex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_CURSOR_MOVE);
        invalidmovetex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_CURSOR_DENIED);
        combattex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_CURSOR_ATTACK);
        currentTex = &regulartex;
        EnableContextSwitching();
    }
} // namespace sage
