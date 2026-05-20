#include "MousePicker.hpp"

#include "Camera.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "EngineSystems.hpp"
#include "Settings.hpp"
#include "systems/CollisionSystem.hpp"

#include <algorithm>
#include <limits>

namespace sage
{
    void MousePicker::updateMouseRayCollision()
    {
        resetHitInfo(mouseHitInfo);
        resetHitInfo(navigationHitInfo);

        const auto viewport = sys->settings->GetViewPort();
        ray = GetScreenToWorldRayEx(GetMousePosition(), *sys->camera->getRaylibCam(), viewport.x, viewport.y);
        auto collisions = sys->collisionSystem->GetCollisionsWithRay(ray);

        for (auto it = collisions.begin(); it != collisions.end();)
        {
            if (RequiresMeshCollision(it->collisionLayer))
            {
                if (!findMeshCollision(*it))
                {
                    it = collisions.erase(it);
                    continue;
                }
            }

            ++it;
        }

        if (collisions.empty()) return;

        CollisionSystem::SortCollisionsByDistance(collisions);
        mouseHitInfo = collisions[0];

        if (IsNavigationLayer(mouseHitInfo.collisionLayer))
        {
            navigationHitInfo = mouseHitInfo;
        }
        else
        {
            const auto navIt = std::find_if(collisions.begin(), collisions.end(), [](const CollisionInfo& coll) {
                return IsNavigationLayer(coll.collisionLayer);
            });

            if (navIt != collisions.end())
            {
                navigationHitInfo = *navIt;
            }
        }

        onCollisionHit.Publish(mouseHitInfo.collidedEntityId);
    }

    void MousePicker::resetHitInfo(CollisionInfo& hitInfo)
    {
        hitInfo.collidedEntityId = entt::null;
        hitInfo.collidedBB = {};
        hitInfo.collisionLayer = CollisionLayer{};
        hitInfo.rlCollision = {};
        hitInfo.rlCollision.distance = std::numeric_limits<float>::max();
        hitInfo.rlCollision.hit = false;
    }

    bool MousePicker::findMeshCollision(CollisionInfo& hitInfo) const
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

    void MousePicker::Update()
    {
        updateMouseRayCollision();
    }

    const CollisionInfo& MousePicker::GetMouseHitInfo() const
    {
        return mouseHitInfo;
    }

    const CollisionInfo& MousePicker::GetNavigationHitInfo() const
    {
        return navigationHitInfo;
    }

    const RayCollision& MousePicker::GetFirstNavigationCollision() const
    {
        return navigationHitInfo.rlCollision;
    }

    const RayCollision& MousePicker::GetFirstCollision() const
    {
        return mouseHitInfo.rlCollision;
    }

    MousePicker::MousePicker(entt::registry* _registry, EngineSystems* _sys) : registry(_registry), sys(_sys)
    {
    }
} // namespace sage
