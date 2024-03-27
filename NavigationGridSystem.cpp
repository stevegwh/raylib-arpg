//
// Created by Steve Wheeler on 25/02/2024.
//

#include <queue>
#include <unordered_map>
#include <utility>
#include "NavigationGridSystem.hpp"
#include "Collideable.hpp"
#include "CollisionSystem.hpp"
#include "GameManager.hpp"


namespace sage
{
    /**
     * Translates a world position to a corresponding index on a grid.
     * @param worldPos The position in world space
     * @return The index of the corresponding grid square. -1 if not valid.
     */
    Vector2 NavigationGridSystem::WorldToGridSpace(Vector3 worldPos)
    {
        // Calculate the grid indices for the given world position
        int x = std::floor(worldPos.x / spacing) + (slices / 2);
        int y = std::floor(worldPos.z / spacing) + (slices / 2);
        
        return {static_cast<float>(x), static_cast<float>(y)};
    }
    
    void NavigationGridSystem::Init(int _slices, float _spacing)
    {
        slices = _slices;
        spacing = _spacing;
        
        int halfSlices = slices / 2;
    
        // Resize gridSquares to the appropriate size
        gridSquares.clear();
        gridSquares.resize(slices);
        for (int i = 0; i < slices; ++i) {
            gridSquares[i].resize(slices);
        }
    
        for (int j = -halfSlices; j < halfSlices; j++)
        {
            for (int i = -halfSlices; i < halfSlices; i++)
            {
                Vector3 v1 = {(float)i * spacing, 0, (float)j * spacing};
                Vector3 v3 = {(float)(i + 1) * spacing, 1.0f, (float)(j + 1) * spacing};
    
                EntityID id = Registry::GetInstance().CreateEntity();
    
                auto gridSquare = std::make_unique<NavigationGridSquare>(id,
                                                                         (Vector2){ .x = static_cast<float>(i + halfSlices), 
                                                                                    .y = static_cast<float>(j + halfSlices)},
                                                                         v1,
                                                                         v3,
                                                                         Vector3Subtract(v1, v3));
                // Store grid square in the 2D array
                gridSquares[j + halfSlices][i + halfSlices] = gridSquare.get();
                AddComponent(std::move(gridSquare));
            }
        }
        
    }
    
    
    
    /**
     * Generates a sequence of nodes that should be the "optimal" route from point A to point B.
     * @return A vector of "nodes" to travel to in sequential order
     */
    std::vector<Vector3> NavigationGridSystem::Pathfind(const Vector3& startPos, const Vector3& finishPos)
    {
        auto startGridSquare = WorldToGridSpace(startPos);
        auto finishGridSquare = WorldToGridSpace(finishPos);

        int startrow = startGridSquare.y;
        int startcol = startGridSquare.x;

        int finishrow = finishGridSquare.y;
        int finishcol = finishGridSquare.x;

        int W = gridSquares.at(0).size();
        int H = gridSquares.size();
        auto inside = [&](int row, int col) { return 0 <= row && row < H && 0 <= col && col < W; };

        std::vector<std::vector<bool>> visited(H, std::vector<bool>(W, false));
        std::vector<std::vector<std::pair<int, int>>> came_from(H, std::vector<std::pair<int, int>>(W, std::pair<int, int>(-1, -1)));

        std::vector<std::pair<int, int>> directions = { {1,0}, {0,1}, {-1,0}, {0,-1}, {1,1}, {-1,1}, {-1,-1}, {1,-1} };
        std::queue<std::pair<int,int>> frontier;

        frontier.emplace(startrow, startcol);
        visited[startrow][startcol] = true;

        bool pathFound = false;

        while (!frontier.empty())
        {
            auto current = frontier.front();
            frontier.pop();

            if (current.first == finishrow && current.second == finishcol) {
                pathFound = true;
                break;
            }

            for (const auto& dir : directions)
            {
                int next_row = current.first + dir.first;
                int next_col = current.second + dir.second;

                if (inside(next_row, next_col) && !visited[next_row][next_col] && !gridSquares[next_row][next_col]->occupied)
                {
                    frontier.emplace(next_row, next_col);
                    visited[next_row][next_col] = true;
                    came_from[next_row][next_col] = current;
                }
            }
        }
        
        if (!pathFound) {
            return {}; 
        }

        // Trace path back from finish to start
        std::vector<Vector3> path;
        std::pair<int, int> current = {finishrow, finishcol};
        while (current.first != startrow || current.second != startcol)
        {
            auto node = gridSquares[current.first][current.second];
            path.push_back(node->worldPosMin);
            current = came_from[current.first][current.second];
        }
        std::reverse(path.begin(), path.end());

        return path;
    }


void NavigationGridSystem::PopulateGrid()
    {
        for (auto& row : gridSquares)
        {
            for (auto& gridSquare : row)
            {
                gridSquare->occupied = false;
            }
        }
    
        CollisionSystem& collisionSystem = *GM.collisionSystem;
        const auto& collisionComponents = collisionSystem.GetComponents();
        for (const auto& bb : collisionComponents)
        {
            if (bb.second->collisionLayer != BUILDING) continue;
    
            // Get the grid indices for the bounding box
            Vector2 topLeftIndex = WorldToGridSpace(bb.second->worldBoundingBox.min);
            Vector2 bottomRightIndex = WorldToGridSpace(bb.second->worldBoundingBox.max);

            int min_col = std::min((int)topLeftIndex.x, (int)bottomRightIndex.x);
            int max_col = std::max((int)topLeftIndex.x, (int)bottomRightIndex.x);
            int min_row = std::min((int)topLeftIndex.y, (int)bottomRightIndex.y);
            int max_row = std::max((int)topLeftIndex.y, (int)bottomRightIndex.y);


            for (int row = min_row; row <= max_row; ++row)
            {
                for (int col = min_col; col <= max_col; ++col)
                {
                    // Access grid square from the 2D array
                    gridSquares[row][col]->occupied = true;
                }
            }

        }
    }
    
    const std::vector<std::vector<NavigationGridSquare*>>& NavigationGridSystem::GetGridSquares()
    {
        return gridSquares;
    }

} // sage