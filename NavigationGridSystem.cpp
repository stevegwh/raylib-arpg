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
        // Calculate the grid indices for the given world position
        int x = std::floor(worldPos.x / spacing) + (slices / 2);
        int y = std::floor(worldPos.z / spacing) + (slices / 2);

        // Ensure the indices are within bounds
        if (y < 0 || y >= slices || x < 0 || x >= slices) {
            return -1; // Return -1 for positions outside the grid
        }

        // Calculate the array index corresponding to the grid position
        int idx = x * slices + y;

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
                
                EntityID id = Registry::GetInstance().CreateEntity();
                
                auto gridSquare = std::make_unique<NavigationGridSquare>(id, 
                                                                         ((i + halfSlices) * slices) + (j + halfSlices),
                                                                         v1,
                                                                         v3,
                                                                         Vector3Subtract(v3, v1));
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
        
        int startGridSquare = WorldToGridSpace(startPos);
        int finishGridSquare = WorldToGridSpace(finishPos);

        // Find where start/finish are in terms of grid position.
        // I assume just modulo to grid dimensions

        // Flood fill algorithm

        return nodes;
    }

    void NavigationGridSystem::PopulateGrid()
    {
        for (auto& gridSquare : gridSquares)
        {
            gridSquare->occupied = false;
        }
    
        CollisionSystem& collisionSystem = *Game::GetInstance().collisionSystem;
        const auto& collisionComponents = collisionSystem.GetComponents();
        for (const auto& bb : collisionComponents)
        {
            if (bb.second->collisionLayer != BUILDING) continue;
    
            int topleftidx = WorldToGridSpace(bb.second->worldBoundingBox.min);
            int bottomrightidx = WorldToGridSpace(bb.second->worldBoundingBox.max);
    
            int min_x = std::min(topleftidx % slices, bottomrightidx % slices);
            int max_x = std::max(topleftidx % slices, bottomrightidx % slices);
            int min_y = std::min(topleftidx / slices, bottomrightidx / slices);
            int max_y = std::max(topleftidx / slices, bottomrightidx / slices);
    
            for (int y = min_y; y <= max_y; ++y)
            {
                for (int x = min_x; x <= max_x; ++x)
                {
                    int idx = y * slices + x;
                    gridSquares[idx]->occupied = true;
                }
            }
        }
    }



    const std::vector<NavigationGridSquare*>& NavigationGridSystem::GetGridSquares()
    {
        return gridSquares;
    }

} // sage