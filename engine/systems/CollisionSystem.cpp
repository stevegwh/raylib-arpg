//
// Created by Steve Wheeler on 18/02/2024.
//
#include "CollisionSystem.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include <Serializer.hpp>

#include <algorithm>
#include <utility>

// TODO: Decouple this from LeverQuest

namespace sage
{
    CollisionMask CollisionSystem::ResolveQueryMask(const CollisionLayer layer) const
    {
        if (layer == collision_layers::Default) return defaultQueryMask;
        return GetDefaultCollisionMask(layer);
    }

    void CollisionSystem::Update() const
    {
        // Static collideables opt out via the static bool — their worldBoundingBox
        // was baked at construction and never needs recomputing unless a
        // transient static override asks us to refresh it anyway.
        auto view = registry->view<sgTransform, Collideable>();
        view.each([this](const entt::entity entity, const sgTransform& t, Collideable& c) {
            const bool forceRefresh = registry->any_of<CollideableStaticOverride>(entity);
            if (c.isStatic && !forceRefresh) return;
            c.worldBoundingBox = forceRefresh ? TransformBoundingBoxByCorners(c.localBoundingBox, t.GetMatrix())
                                              : TransformBoundingBox(c.localBoundingBox, t.GetMatrixNoRot());
        });
    }

    void CollisionSystem::SortCollisionsByDistance(std::vector<CollisionInfo>& collisions)
    {
        std::ranges::sort(collisions, [](const CollisionInfo& a, const CollisionInfo& b) {
            return a.rlCollision.distance < b.rlCollision.distance;
        });
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithBoundingBox(
        const BoundingBox& bb, CollisionLayer layer)
    {
        return GetCollisionsWithBoundingBox(bb, ResolveQueryMask(layer));
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithBoundingBox(
        const BoundingBox& bb, CollisionMask mask)
    {
        std::vector<CollisionInfo> collisions;
        const auto view = registry->view<Collideable>();
        view.each([&](auto entity, const auto& c) {
            if (!c.active) return;
            if (mask.Contains(c.collisionLayer))
            {
                if (CheckCollisionBoxes(bb, c.worldBoundingBox))
                {
                    const CollisionInfo info = {
                        .collidedEntityId = entity,
                        .collidedBB = c.worldBoundingBox,
                        .rlCollision = {},
                        .collisionLayer = c.collisionLayer};
                    collisions.push_back(info);
                }
            }
        });
        SortCollisionsByDistance(collisions);
        return collisions;
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(const Ray& ray, CollisionLayer layer)
    {
        return GetCollisionsWithRay(entt::null, ray, layer);
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(const Ray& ray, CollisionMask mask)
    {
        return GetCollisionsWithRay(entt::null, ray, mask);
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(
        const entt::entity& caster, const Ray& ray, CollisionLayer layer)
    {
        return GetCollisionsWithRay(caster, ray, ResolveQueryMask(layer));
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(
        const entt::entity& caster, const Ray& ray, CollisionMask mask)
    {
        std::vector<CollisionInfo> collisions;

        const auto view = registry->view<Collideable>();

        view.each([&](auto entity, const auto& c) {
            if (!c.active || entity == caster) return;
            if (mask.Contains(c.collisionLayer))
            {
                auto col = GetRayCollisionBox(ray, c.worldBoundingBox);
                if (col.hit)
                {
                    const CollisionInfo info = {
                        .collidedEntityId = entity,
                        .collidedBB = c.worldBoundingBox,
                        .rlCollision = col,
                        .collisionLayer = c.collisionLayer};
                    collisions.push_back(info);
                }
            }
        });
        SortCollisionsByDistance(collisions);
        return collisions;
    }

    bool CollisionSystem::GetFirstCollisionWithRay(const Ray& ray, CollisionInfo& info, CollisionLayer layer) const
    {
        return GetFirstCollisionWithRay(ray, info, ResolveQueryMask(layer));
    }

    bool CollisionSystem::GetFirstCollisionWithRay(const Ray& ray, CollisionInfo& info, CollisionMask mask) const
    {
        for (const auto view = registry->view<Collideable>(); const auto& entity : view)
        {
            const auto& c = registry->get<Collideable>(entity);
            if (!c.active) continue;
            if (mask.Contains(c.collisionLayer))
            {
                const auto col = GetRayCollisionBox(ray, c.worldBoundingBox);
                if (col.hit)
                {
                    const CollisionInfo _info = {
                        .collidedEntityId = entity,
                        .collidedBB = c.worldBoundingBox,
                        .rlCollision = col,
                        .collisionLayer = c.collisionLayer};
                    info = _info;
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<CollisionInfo> CollisionSystem::GetMeshCollisionsWithRay(
        const entt::entity& caster, const Ray& ray, CollisionLayer layer)
    {
        return GetMeshCollisionsWithRay(caster, ray, ResolveQueryMask(layer));
    }

    std::vector<CollisionInfo> CollisionSystem::GetMeshCollisionsWithRay(
        const entt::entity& caster, const Ray& ray, CollisionMask mask)
    {
        std::vector<CollisionInfo> collisions;
        const auto view = registry->view<Collideable>();
        view.each([&](auto entity, const auto& c) {
            if (!c.active || entity == caster) return;
            if (mask.Contains(c.collisionLayer))
            {
                if (registry->any_of<Renderable>(entity))
                {
                    auto& renderable = registry->get<Renderable>(entity);
                    auto& transform = registry->get<sgTransform>(entity);
                    auto col = renderable.GetModel()->GetRayMeshCollision(ray, 0, transform.GetMatrix());
                    if (col.hit)
                    {
                        const CollisionInfo info = {
                            .collidedEntityId = entity,
                            .collidedBB = c.worldBoundingBox,
                            .rlCollision = col,
                            .collisionLayer = c.collisionLayer};
                        collisions.push_back(info);
                    }
                }
            }
        });
        SortCollisionsByDistance(collisions);
        return collisions;
    }

    void CollisionSystem::DrawDebug() const
    {
        const auto view = registry->view<Collideable>();
        for (const auto entity : view)
        {
            const auto& c = registry->get<Collideable>(entity);
            if (!c.active) continue;
            if (collision_masks::Navigation.Contains(c.collisionLayer))
            {
                auto col = YELLOW;
                if (c.collisionLayer == collision_layers::GeometryComplex ||
                    c.collisionLayer == collision_layers::GeometrySimple)
                {
                    col = GREEN;
                }
                DrawBoundingBox(c.worldBoundingBox, col);
            }
        }
    }

    void CollisionSystem::BoundingBoxDraw(entt::entity entityId, Color color) const
    {
        auto& col = registry->get<Collideable>(entityId);
        Vector3 min = col.worldBoundingBox.min;
        Vector3 max = col.worldBoundingBox.max;

        // Calculate the center of the bounding box
        Vector3 center = {(min.x + max.x) / 2, (min.y + max.y) / 2, (min.z + max.z) / 2};

        // Calculate dimensions
        float width = max.x - min.x;
        float height = max.y - min.y;
        float depth = max.z - min.z;

        color.a = 100;

        // Draw the cube at the calculated center with the correct dimensions
        DrawCube(center, width, height, depth, color);
    }

    bool CollisionSystem::CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2)
    {
        return CheckCollisionBoxes(col1, col2);
    }

    bool CollisionSystem::GetFirstCollisionBB(
        entt::entity caller, BoundingBox bb, CollisionLayer layer, CollisionInfo& out)
    {
        return GetFirstCollisionBB(caller, bb, ResolveQueryMask(layer), out);
    }

    bool CollisionSystem::GetFirstCollisionBB(
        entt::entity caller, BoundingBox bb, CollisionMask mask, CollisionInfo& out) const
    {
        auto view = registry->view<Collideable>();

        for (const auto& entity : view)
        {
            if (caller == entity) continue;
            const auto& col = view.get<Collideable>(entity);
            if (!col.active) continue;
            if (mask.Contains(col.collisionLayer))
            {
                const bool colHit = CheckBoxCollision(bb, col.worldBoundingBox);
                if (colHit)
                {
                    CollisionInfo colInfo = {
                        .collidedEntityId = entity,
                        .collidedBB = col.worldBoundingBox,
                        .rlCollision = {},
                        .collisionLayer = col.collisionLayer};
                    out = colInfo;
                    return true;
                };
            }
        }
        return false;
    }

    void CollisionSystem::SetDefaultQueryMask(const CollisionMask mask)
    {
        defaultQueryMask = mask;
    }

    CollisionSystem::CollisionSystem(entt::registry* _registry) : registry(_registry)
    {
    }
} // namespace sage
