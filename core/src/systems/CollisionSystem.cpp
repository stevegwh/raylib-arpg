#include "CollisionSystem.hpp"
//
// Created by Steve Wheeler on 18/02/2024.
//
#include "CollisionSystem.hpp"

#include "Application.hpp"
#include "components/Renderable.hpp"
#include <Serializer.hpp>

#include <algorithm>

bool compareRayCollisionDistances(
    const sage::CollisionInfo& a, const sage::CollisionInfo& b)
{
    return a.rlCollision.distance < b.rlCollision.distance;
}

namespace sage
{
    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithBoundingBox(
        const BoundingBox& bb, CollisionLayer layer)
    {
        std::vector<CollisionInfo> collisions;

        auto view = registry->view<Collideable>();

        view.each([&](auto entity, const auto& c) {
            if (collisionMatrix[static_cast<int>(layer)]
                               [static_cast<int>(c.collisionLayer)])
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

        std::sort(collisions.begin(), collisions.end(), compareRayCollisionDistances);

        return collisions;
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(
        const Ray& ray, CollisionLayer layer)
    {
        return GetCollisionsWithRay(entt::null, ray, layer);
    }

    bool CollisionSystem::GetFirstCollisionWithRay(
        const Ray& ray, CollisionInfo& info, CollisionLayer layer)
    {
        auto view = registry->view<Collideable>();

        view.each([&](auto entity, const auto& c) {
            if (collisionMatrix[static_cast<int>(layer)]
                               [static_cast<int>(c.collisionLayer)])
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
            if (collisionMatrix[static_cast<int>(layer)]
                               [static_cast<int>(c.collisionLayer)])
            {
                if (registry->any_of<Renderable>(entity))
                {
                    auto& r = registry->get<Renderable>(entity);
                    auto mesh = *r.model.meshes;
                    auto col = GetRayCollisionMesh(ray, mesh, r.model.transform);
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

        std::sort(collisions.begin(), collisions.end(), compareRayCollisionDistances);

        return collisions;
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(
        const entt::entity& caster, const Ray& ray, CollisionLayer layer)
    {
        std::vector<CollisionInfo> collisions;

        auto view = registry->view<Collideable>();

        view.each([&](auto entity, const auto& c) {
            if (entity == caster) return;
            if (collisionMatrix[static_cast<int>(layer)]
                               [static_cast<int>(c.collisionLayer)])
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

        std::sort(collisions.begin(), collisions.end(), compareRayCollisionDistances);

        return collisions;
    }

    /**
     * Responsible for updating the collideable when its corresponding transform changes
     * @param entity
     **/
    void CollisionSystem::OnTransformUpdate(entt::entity entity)
    {
        auto& trans = registry->get<sgTransform>(entity);
        auto& col = registry->get<Collideable>(entity);
        Matrix mat = trans.GetMatrixNoRot();
        auto bb = col.localBoundingBox;
        bb.min = Vector3Transform(bb.min, mat);
        bb.max = Vector3Transform(bb.max, mat);
        col.worldBoundingBox = bb;
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

        // Draw the cube at the calculated center with the correct dimensions
        DrawCube(center, width, height, depth, color);
    }

    /**
     * Calculates worldBoundingBox by multiplying localBoundingBox with the passed
     * transform matrix
     * @param entityId The id of the entity
     * @param mat The transform matrix for the local bounding box
     */
    void CollisionSystem::UpdateWorldBoundingBox(
        entt::entity entityId, Matrix mat) // TODO: I don't like the name
    {
        registry->patch<Collideable>(entityId, [mat](auto& col) {
            auto bb = col.localBoundingBox;
            bb.min = Vector3Transform(bb.min, mat);
            bb.max = Vector3Transform(bb.max, mat);
            col.worldBoundingBox = bb;
        });
    }

    bool CollisionSystem::CheckBoxCollision(
        const BoundingBox& col1, const BoundingBox& col2)
    {
        return CheckCollisionBoxes(col1, col2);
    }

    bool CollisionSystem::GetFirstCollision(entt::entity entity)
    // TODO: Terrible name. Should be "CheckAnyBoxCollision"
    {
        const auto& targetCol = registry->get<Collideable>(entity);

        auto view = registry->view<Collideable>();

        for (const auto& ent : view)
        {
            const auto& c = view.get<Collideable>(ent);
            if (c.collisionLayer != CollisionLayer::BUILDING) continue;
            // TODO: Wanted to query a collision matrix but is far too slow
            bool colHit =
                CheckBoxCollision(targetCol.worldBoundingBox, c.worldBoundingBox);
            if (colHit) return true;
        }
        return false;
    }

    CollisionMatrix CollisionSystem::CreateCollisionMatrix()
    {
        int numLayers = static_cast<int>(CollisionLayer::COUNT);
        std::vector<std::vector<bool>> matrix(
            numLayers, std::vector<bool>(numLayers, false));

        matrix[static_cast<int>(CollisionLayer::DEFAULT)]
              [static_cast<int>(CollisionLayer::PLAYER)] = true;
        matrix[static_cast<int>(CollisionLayer::DEFAULT)]
              [static_cast<int>(CollisionLayer::ENEMY)] = true;
        matrix[static_cast<int>(CollisionLayer::DEFAULT)]
              [static_cast<int>(CollisionLayer::NPC)] = true;
        // matrix[static_cast<int>(CollisionLayer::DEFAULT)][static_cast<int>(CollisionLayer::NAVIGATION)]
        // = true;
        matrix[static_cast<int>(CollisionLayer::DEFAULT)]
              [static_cast<int>(CollisionLayer::BUILDING)] = true;
        matrix[static_cast<int>(CollisionLayer::DEFAULT)]
              [static_cast<int>(CollisionLayer::FLOOR)] = true;

        matrix[static_cast<int>(CollisionLayer::PLAYER)]
              [static_cast<int>(CollisionLayer::ENEMY)] = true;
        matrix[static_cast<int>(CollisionLayer::PLAYER)]
              [static_cast<int>(CollisionLayer::BUILDING)] = true;

        matrix[static_cast<int>(CollisionLayer::ENEMY)]
              [static_cast<int>(CollisionLayer::PLAYER)] = true;
        matrix[static_cast<int>(CollisionLayer::ENEMY)]
              [static_cast<int>(CollisionLayer::BUILDING)] = true;

        matrix[static_cast<int>(CollisionLayer::BOYD)]
              [static_cast<int>(CollisionLayer::PLAYER)] = true;
        matrix[static_cast<int>(CollisionLayer::BOYD)]
              [static_cast<int>(CollisionLayer::NPC)] = true;
        matrix[static_cast<int>(CollisionLayer::BOYD)]
              [static_cast<int>(CollisionLayer::ENEMY)] = true;

        matrix[static_cast<int>(CollisionLayer::NAVIGATION)]
              [static_cast<int>(CollisionLayer::FLOOR)] = true;

        return matrix;
    }

    CollisionSystem::CollisionSystem(entt::registry* _registry) : BaseSystem(_registry)
    {
        collisionMatrix = CreateCollisionMatrix();
    }
} // namespace sage
