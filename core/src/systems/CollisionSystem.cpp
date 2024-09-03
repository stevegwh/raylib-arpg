//
// Created by Steve Wheeler on 18/02/2024.
//
#include "CollisionSystem.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include <Serializer.hpp>

#include <algorithm>

namespace sage
{

    void CollisionSystem::SortCollisionsByDistance(std::vector<CollisionInfo>& collisions)
    {
        std::sort(collisions.begin(), collisions.end(), [](const CollisionInfo& a, const CollisionInfo& b) {
            return a.rlCollision.distance < b.rlCollision.distance;
        });
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithBoundingBox(
        const BoundingBox& bb, CollisionLayer layer)
    {
        std::vector<CollisionInfo> collisions;

        auto view = registry->view<Collideable>();

        view.each([&](auto entity, const auto& c) {
            if (collisionMatrix[static_cast<int>(layer)][static_cast<int>(c.collisionLayer)])
            {
                if (CheckCollisionBoxes(bb, c.worldBoundingBox))
                {
                    CollisionInfo info = {
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

    bool CollisionSystem::GetFirstCollisionWithRay(const Ray& ray, CollisionInfo& info, CollisionLayer layer)
    {
        auto view = registry->view<Collideable>();

        view.each([&](auto entity, const auto& c) {
            if (collisionMatrix[static_cast<int>(layer)][static_cast<int>(c.collisionLayer)])
            {
                auto col = GetRayCollisionBox(ray, c.worldBoundingBox);
                if (col.hit)
                {
                    CollisionInfo _info = {
                        .collidedEntityId = entity,
                        .collidedBB = c.worldBoundingBox,
                        .rlCollision = col,
                        .collisionLayer = c.collisionLayer};
                    info = _info;
                }
            }
        });

        return false;
    }

    std::vector<CollisionInfo> CollisionSystem::GetMeshCollisionsWithRay(
        const entt::entity& caster, const Ray& ray, CollisionLayer layer)
    {
        std::vector<CollisionInfo> collisions;

        auto view = registry->view<Collideable>();

        view.each([&](auto entity, const auto& c) {
            if (entity == caster) return;
            if (collisionMatrix[static_cast<int>(layer)][static_cast<int>(c.collisionLayer)])
            {
                if (registry->any_of<Renderable>(entity))
                {
                    auto& renderable = registry->get<Renderable>(entity);
                    auto col = renderable.GetModel()->GetRayMeshCollision(ray, 0);
                    if (col.hit)
                    {
                        CollisionInfo info = {
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

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(
        const entt::entity& caster, const Ray& ray, CollisionLayer layer)
    {
        std::vector<CollisionInfo> collisions;

        auto view = registry->view<Collideable>();

        view.each([&](auto entity, const auto& c) {
            if (entity == caster) return;
            if (collisionMatrix[static_cast<int>(layer)][static_cast<int>(c.collisionLayer)])
            {
                auto col = GetRayCollisionBox(ray, c.worldBoundingBox);
                if (col.hit)
                {
                    CollisionInfo info = {
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

    void CollisionSystem::DrawDebug()
    {
        auto view = registry->view<Collideable>();
        for (auto entity : view)
        {
            auto& c = registry->get<Collideable>(entity);
            if (c.debugDraw)
            {
                DrawBoundingBox(c.worldBoundingBox, YELLOW);
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

    /**
     * Calculates worldBoundingBox by multiplying localBoundingBox with the passed
     * transform matrix
     * @param entityId The id of the entity
     * @param mat The transform matrix for the local bounding box
     */
    void CollisionSystem::UpdateWorldBoundingBox(entt::entity entityId, Matrix mat) // TODO: I don't like the name
    {
        registry->patch<Collideable>(entityId, [mat](auto& col) {
            auto bb = col.localBoundingBox;
            bb.min = Vector3Transform(bb.min, mat);
            bb.max = Vector3Transform(bb.max, mat);
            col.worldBoundingBox = bb;
        });
    }

    bool CollisionSystem::CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2)
    {
        return CheckCollisionBoxes(col1, col2);
    }

    bool CollisionSystem::GetFirstCollisionBB(
        entt::entity caller, BoundingBox bb, CollisionLayer layer, CollisionInfo& out)
    {
        auto view = registry->view<Collideable>();

        for (const auto& entity : view)
        {
            if (caller == entity) continue;
            const auto& col = view.get<Collideable>(entity);
            if (collisionMatrix[static_cast<int>(layer)][static_cast<int>(col.collisionLayer)])
            {
                bool colHit = CheckBoxCollision(bb, col.worldBoundingBox);
                if (colHit)
                {
                    CollisionInfo colInfo;
                    colInfo = {
                        .collidedEntityId = entity,
                        .collidedBB = col.worldBoundingBox,
                        .rlCollision = {},
                        .collisionLayer = layer};
                    out = colInfo;
                    return true;
                };
            }
        }
        return false;
    }

    CollisionMatrix CollisionSystem::CreateCollisionMatrix()
    {
        int numLayers = static_cast<int>(CollisionLayer::COUNT);
        std::vector<std::vector<bool>> matrix(numLayers, std::vector<bool>(numLayers, false));

        matrix[static_cast<int>(CollisionLayer::DEFAULT)][static_cast<int>(CollisionLayer::PLAYER)] = true;
        matrix[static_cast<int>(CollisionLayer::DEFAULT)][static_cast<int>(CollisionLayer::ENEMY)] = true;
        matrix[static_cast<int>(CollisionLayer::DEFAULT)][static_cast<int>(CollisionLayer::NPC)] = true;
        // matrix[static_cast<int>(CollisionLayer::DEFAULT)][static_cast<int>(CollisionLayer::NAVIGATION)]
        // = true;
        matrix[static_cast<int>(CollisionLayer::DEFAULT)][static_cast<int>(CollisionLayer::BUILDING)] = true;
        matrix[static_cast<int>(CollisionLayer::DEFAULT)][static_cast<int>(CollisionLayer::FLOOR)] = true;

        matrix[static_cast<int>(CollisionLayer::PLAYER)][static_cast<int>(CollisionLayer::ENEMY)] = true;
        matrix[static_cast<int>(CollisionLayer::PLAYER)][static_cast<int>(CollisionLayer::BUILDING)] = true;

        matrix[static_cast<int>(CollisionLayer::ENEMY)][static_cast<int>(CollisionLayer::PLAYER)] = true;
        matrix[static_cast<int>(CollisionLayer::ENEMY)][static_cast<int>(CollisionLayer::BUILDING)] = true;

        matrix[static_cast<int>(CollisionLayer::BOYD)][static_cast<int>(CollisionLayer::PLAYER)] = true;
        matrix[static_cast<int>(CollisionLayer::BOYD)][static_cast<int>(CollisionLayer::NPC)] = true;
        matrix[static_cast<int>(CollisionLayer::BOYD)][static_cast<int>(CollisionLayer::ENEMY)] = true;

        matrix[static_cast<int>(CollisionLayer::NAVIGATION)][static_cast<int>(CollisionLayer::FLOOR)] = true;

        return matrix;
    }

    void CollisionSystem::Update()
    {
        auto view = registry->view<Collideable, CollisionChecker>();
        for (entt::entity entity : view)
        {
            CollisionInfo out;
            auto& colChecker = registry->get<CollisionChecker>(entity);
            if (Vector3Distance(colChecker.origin, colChecker.destination) > colChecker.maxDistance)
            {
                colChecker.onHit.publish(entity, out);
                registry->remove<CollisionChecker>(entity);
                continue;
            }
            const auto& col = registry->get<Collideable>(entity);
            auto& bb = col.worldBoundingBox;
            if (GetFirstCollisionBB(entity, bb, col.collisionLayer, out))
            {
                colChecker.onHit.publish(entity, out);
            }
        }
    }

    CollisionSystem::CollisionSystem(entt::registry* _registry) : BaseSystem(_registry)
    {
        collisionMatrix = CreateCollisionMatrix();
    }
} // namespace sage
