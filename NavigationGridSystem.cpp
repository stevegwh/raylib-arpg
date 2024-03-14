//
// Created by Steve Wheeler on 25/02/2024.
//

#include "NavigationGridSystem.hpp"
#include "Collideable.hpp"
#include "CollisionSystem.hpp"
#include "Game.hpp"


namespace sage
{
    /**
     * Translates a world position to a corresponding index on a grid.
     * @param worldPos The position in world space
     * @return The index of the corresponding grid square. -1 if not valid.
     */
    int NavigationGridSystem::WorldToGridSpace(Vector3 worldPos)
    {
        // Grid's centre is 0,0.
        // 'Translate' grid's centre to 0,0 then translate back after?
        int x = std::floor(worldPos.z) / spacing;
        int y = std::floor(worldPos.x) / spacing;
        // 0,0 in world space is the centre of the grid. Therefore, need to offset the position by half of the grid.
        // + (slices/2) is likely a hack and not needed if I calculate x/y better, but (slices/2) is just the width of one grid space.
        int offset = (slices * slices) / 2 + (slices/2);
        int idx = (x * slices + y) + offset;
        
        return idx;
    }

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
                
                auto gridSquare = std::make_unique<NavigationGridSquare>(id, gridSquares.size() + 1);
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
    std::vector<Vector3> NavigationGridSystem::Pathfind(const Vector3& startPos, const Vector3& finishPos
    , EntityID startGridId, EntityID finishGridId)
    {
        std::vector<Vector3> nodes;
        
        const NavigationGridSquare* const startGridSquare = GetComponent(startGridId);
        const NavigationGridSquare* const finishGridSquare = GetComponent(finishGridId);

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