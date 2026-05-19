#include "EditorPickingService.hpp"

#include "EditorTransformMath.hpp"
#include "engine/Camera.hpp"
#include "engine/CollisionLayers.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/Settings.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/CollisionSystem.hpp"

#include <limits>
#include <vector>

namespace sage::editor
{
    EditorPickingService::EditorPickingService(EngineSystems* _sys) : sys(_sys)
    {
    }

    std::optional<entt::entity> EditorPickingService::PickSceneEntity(
        const Vector2 screenPosition,
        const entt::entity ignoredEntity) const
    {
        const auto viewport = sys->settings->GetViewPort();
        const auto ray = GetScreenToWorldRayEx(screenPosition, *sys->camera->getRaylibCam(), viewport.x, viewport.y);
        auto collisions = sys->collisionSystem->GetCollisionsWithRay(ray, CollisionMask{~0ull});

        std::vector<CollisionInfo> objectHits;
        std::vector<CollisionInfo> fallbackHits;

        for (auto collision : collisions)
        {
            const auto entity = collision.collidedEntityId;
            if (entity == entt::null || entity == ignoredEntity || !sys->registry->valid(entity) ||
                !sys->registry->any_of<sgTransform>(entity))
            {
                continue;
            }

            if (sys->registry->any_of<Renderable>(entity))
            {
                const auto& renderable = sys->registry->get<Renderable>(entity);
                const auto* model = renderable.GetModel();
                if (model == nullptr) continue;

                const auto& transform = sys->registry->get<sgTransform>(entity);
                const Matrix entityMatrix =
                    BuildRenderableEntityMatrix(transform.GetWorldPos(), transform.GetWorldRot(), transform.GetScale());
                bool meshHit = false;
                RayCollision closestMeshHit{};
                closestMeshHit.distance = std::numeric_limits<float>::max();

                for (int meshIndex = 0; meshIndex < model->GetMeshCount(); ++meshIndex)
                {
                    const auto meshCollision = model->GetRayMeshCollision(ray, meshIndex, entityMatrix);
                    if (meshCollision.hit && meshCollision.distance < closestMeshHit.distance)
                    {
                        closestMeshHit = meshCollision;
                        meshHit = true;
                    }
                }

                if (!meshHit) continue;
                collision.rlCollision = closestMeshHit;
            }

            if (IsNavigationLayer(collision.collisionLayer))
            {
                fallbackHits.push_back(collision);
            }
            else
            {
                objectHits.push_back(collision);
            }
        }

        auto selectClosest = [](std::vector<CollisionInfo>& hits) -> std::optional<entt::entity> {
            if (hits.empty()) return std::nullopt;
            CollisionSystem::SortCollisionsByDistance(hits);
            return hits.front().collidedEntityId;
        };

        if (auto object = selectClosest(objectHits); object.has_value()) return object;
        return selectClosest(fallbackHits);
    }
} // namespace sage::editor
