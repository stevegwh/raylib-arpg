//
// Created by Steve Wheeler on 25/02/2024.
//

#include "NavigationGridSystem.hpp"
#include "Collideable.hpp"
#include "CollisionSystem.hpp"
#include "Game.hpp"

namespace sage
{

    inline void NavigationGridSystem::init(CollisionSystem& collisionSystem)
    {
        int halfSlices = slices / 2;

        for (int i = -halfSlices; i < halfSlices; i++) 
        {
            for (int j = -halfSlices; j < halfSlices; j++) 
            {
                Vector3 v1 = {(float)i * spacing, 0, (float)j * spacing};
                Vector3 v3 = {(float)(i + 1) * spacing, 1.0f, (float)(j + 1) * spacing};

                BoundingBox boundingBox;
                boundingBox.min = v1;
                boundingBox.max = v3; // Diagonal opposite

                // Padding
                float padding = 0.01f;
                boundingBox.min.x += padding;
                boundingBox.min.z += padding;
                boundingBox.max.x -= padding;
                boundingBox.max.z -= padding;
                
                EntityID id = Registry::GetInstance().CreateEntity();
                
                // Collisions necessary to see if grid square is occupied or not
                auto collideable = std::make_unique<Collideable>(Collideable(id, boundingBox));
                collideable->collisionLayer = NAVIGATION;
                collisionSystem.AddComponent(std::move(collideable));
                
                auto gridSquare = std::make_unique<NavigationGridSquare>(id);
                // Store grid squares in an ordered way so we can navigate
                gridSquares.push_back(gridSquare.get());
                AddComponent(std::move(gridSquare));

            }
        }
    }

    /**
     * Generates a sequence of nodes that should be the "optimal" route from point A to point B.
     * @return A vector of "nodes" to travel to in sequential order
     */
    std::vector<Vector3> NavigationGridSystem::Pathfind(const Vector3& start, const Vector3& finish)
    {
        std::vector<Vector3> nodes;

        // gridSquares 1000.
        int x = static_cast<int>(start.x) % static_cast<int>(spacing);
        int y = static_cast<int>(start.y) % static_cast<int>(spacing);
        int i = x * spacing + y * spacing;

        // Find where start/finish are in terms of grid position.
        // I assume just modulo to grid dimensions

        // Flood fill algorithm

        return nodes;
    }

    void NavigationGridSystem::PopulateGrid()
    {
        CollisionSystem& collisionSystem = *Game::GetInstance().collisionSystem;
        for (auto& gridSquare : gridSquares)
        {
            gridSquare->occupied = collisionSystem.GetFirstCollision(gridSquare->entityId);
        }
    }

    const std::vector<NavigationGridSquare*>& NavigationGridSystem::GetGridSquares()
    {
        return gridSquares;
    }

} // sage