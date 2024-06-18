//
// Created by Steve Wheeler on 25/02/2024.
//

#include "NavigationGridSystem.hpp"
#include "components/ControllableActor.hpp"
#include "components/Transform.hpp"
#include "../utils/PriorityQueue.hpp"

#include <queue>
#include <unordered_map>
#include <utility>
#include <tuple>
#include <iostream>

Vector3 calculateGridsquareCentre(Vector3 min, Vector3 max)
{
    Vector3 size = { 0 };

    size.x = fabsf(max.x - min.x);
    size.y = fabsf(max.y - min.y);
    size.z = fabsf(max.z - min.z);

    return { min.x + size.x/2.0f, min.y + size.y/2.0f, min.z + size.z/2.0f };
}

namespace sage
{

void NavigationGridSystem::DrawDebugPathfinding(const Vector2& minRange, const Vector2& maxRange)
{

    for (int i = 0; i  < gridSquares.size(); i++)
    {
        for (int j = 0; j < gridSquares.at(0).size(); j++)
        {
            gridSquares[i][j]->debugColor = false;
        }
    }
    for (int i = minRange.y; i  < maxRange.y; i++)
    {
        for (int j = minRange.x; j < maxRange.x; j++)
        {
            gridSquares[i][j]->debugColor = true;
        }
    }
}

bool NavigationGridSystem::GetPathfindRange(const entt::entity& actorId, int bounds, Vector2& minRange, Vector2& maxRange)
{
    auto bb = registry->get<Collideable>(actorId).worldBoundingBox;
    Vector3 center = { (bb.min.x + bb.max.x) / 2.0f, (bb.min.y + bb.max.y) / 2.0f, (bb.min.z + bb.max.z) / 2.0f };

    // Calculate the top-left and bottom-right corners of the square grid
    Vector3 topLeft = { center.x - bounds * spacing, center.y, center.z - bounds * spacing };
    Vector3 bottomRight = { center.x + bounds * spacing, center.y, center.z + bounds * spacing };

    // Get the grid indices for the top-left and bottom-right corners
    Vector2 topLeftIndex;
    Vector2 bottomRightIndex;

    bool topLeftValid = WorldToGridSpace(topLeft, topLeftIndex);
    bool bottomRightValid = WorldToGridSpace(bottomRight, bottomRightIndex);
    
    if (!topLeftValid || !bottomRightValid) return false;

    // Clip the top-left and bottom-right indices to the grid boundaries
    topLeftIndex.x = std::max(topLeftIndex.x, 0.0f);
    topLeftIndex.y = std::max(topLeftIndex.y, 0.0f);
    bottomRightIndex.x = std::min(bottomRightIndex.x, static_cast<float>(gridSquares.at(0).size() - 1));
    bottomRightIndex.y = std::min(bottomRightIndex.y, static_cast<float>(gridSquares.size() - 1));

    minRange = { static_cast<float>(topLeftIndex.x), static_cast<float>(topLeftIndex.y) };
    maxRange = { static_cast<float>(bottomRightIndex.x), static_cast<float>(bottomRightIndex.y) };

    return true;
}


/**
 * Translates a world position to a corresponding index on a grid.
 * Checks if the passed position is valid based on the entire grid.
 * @param worldPos The position in world space
 * @out (Out param) The resulting index of the corresponding grid square.
 * @return Whether the move is valid
 */
bool NavigationGridSystem::WorldToGridSpace(Vector3 worldPos, Vector2& out)
{
    return WorldToGridSpace(worldPos, 
                            out, 
                            {0,0}, 
                            {static_cast<float>(gridSquares.at(0).size()), static_cast<float>(gridSquares.size())});
}

/**
 * Translates a world position to a corresponding index on a grid.
 * Checks if the passed position is valid based on a grid range
 * @param worldPos The position in world space
 * @out (Out param) The resulting index of the corresponding grid square.
 * @return Whether the move is valid
 */
bool NavigationGridSystem::WorldToGridSpace(Vector3 worldPos, Vector2& out, const Vector2& minRange, const Vector2& maxRange) const
{
    // Calculate the grid indices for the given world position
    int x = std::floor(worldPos.x / spacing) + (slices / 2);
    int y = std::floor(worldPos.z / spacing) + (slices / 2);
    out = {static_cast<float>(x), static_cast<float>(y)};

    return out.y < maxRange.y && out.x < maxRange.x
    && out.x >= minRange.x && out.y >= minRange.y;
}

void NavigationGridSystem::Init(int _slices, float _spacing)
{
    slices = _slices;
    spacing = _spacing;
    
    int halfSlices = slices / 2;

    // Resize gridSquares to the appropriate size
    gridSquares.clear();
    gridSquares.resize(slices);
    for (int i = 0; i < slices; ++i) 
    {
        gridSquares[i].resize(slices);
    }

    for (int j = -halfSlices; j < halfSlices; j++)
    {
        for (int i = -halfSlices; i < halfSlices; i++)
        {
            Vector3 v1 = {(float)i * spacing, 0, (float)j * spacing};
            Vector3 v3 = {(float)(i + 1) * spacing, 1.0f, (float)(j + 1) * spacing};

            //EntityID id = Registry::GetInstance().CreateEntity();
            entt::entity id = registry->create();

            
            // Store grid square in the 2D array

            auto& gridSquare = registry->emplace<NavigationGridSquare>(id,
                                                                       Vector2{ .x = static_cast<float>(i + halfSlices),
                                                                         .y = static_cast<float>(j + halfSlices)},
                                                                        v1,
                                                                        v3,
                                                                        calculateGridsquareCentre(v1, v3));
            gridSquares[j + halfSlices][i + halfSlices] = &gridSquare;
        }
    }
}

void NavigationGridSystem::DrawDebug() const
{
    for (const auto& gridSquareRow : gridSquares)
    {
        for (const auto& gridSquare : gridSquareRow)
        {
            if (!gridSquare->occupied) continue;
            auto color = RED;
            DrawCubeWires(gridSquare->worldPosCentre,
                          gridSquare->debugBox.x,
                          gridSquare->debugBox.y,
                          gridSquare->debugBox.z,
                          color);
        }
    }
}

/**
 * If actor encounters a non-static obstacle this function resolves it by turning the conflicting actors to the right
 * past the other's bounding box.
 * @param actor 
 * @param obstacle 
 * @param currentDir 
 * @return 
 */
std::vector<Vector3> NavigationGridSystem::ResolveLocalObstacle(entt::entity actor, BoundingBox obstacle, Vector3 currentDir)
{
    Vector2 actorMinIndex, actorMaxIndex, obstacleMinIndex, obstacleMaxIndex;
    auto& actorCollideable = registry->get<Collideable>(actor);
    if (!WorldToGridSpace(actorCollideable.worldBoundingBox.min, actorMinIndex) ||
        !WorldToGridSpace(actorCollideable.worldBoundingBox.max, actorMaxIndex) ||
        !WorldToGridSpace(obstacle.min, obstacleMinIndex) ||
        !WorldToGridSpace(obstacle.max, obstacleMaxIndex))
    {
        return { gridSquares[actorMinIndex.y][actorMinIndex.x]->worldPosMin };
    }
    
    Vector2 rightGridIndex, forwardGridIndex;
    int currentX = std::round(currentDir.x);
    int currentZ = std::round(currentDir.z);
    
    if (currentZ > 0)
    {
        
        if (actorMaxIndex.x <= obstacleMinIndex.x)
            rightGridIndex = { obstacleMinIndex.x - 1, obstacleMaxIndex.y + 1 };
        else
            rightGridIndex = { obstacleMaxIndex.x + 1, obstacleMinIndex.y };
        forwardGridIndex = Vector2Add(rightGridIndex, {0, obstacleMaxIndex.y - obstacleMinIndex.y + 1});
    }
    else if (currentZ < 0)
    {
        if (actorMinIndex.x >= obstacleMaxIndex.x)
            rightGridIndex = { obstacleMaxIndex.x + 1, obstacleMinIndex.y - 1 };
        else
            rightGridIndex = { obstacleMinIndex.x - 1, obstacleMaxIndex.y };
        forwardGridIndex = Vector2Add(rightGridIndex, {0, obstacleMinIndex.y - obstacleMaxIndex.y - 1});
    }

    if (currentX > 0)
    {
        if (actorMaxIndex.y <= obstacleMinIndex.y)
            rightGridIndex = { obstacleMaxIndex.x + 1, obstacleMinIndex.y - 1 };
        else
            rightGridIndex = { obstacleMinIndex.x, obstacleMaxIndex.y + 1 };
        forwardGridIndex = Vector2Add(rightGridIndex, {obstacleMaxIndex.x - obstacleMinIndex.x + 1, 0});
    }
    else if (currentX < 0)
    {
        if (actorMinIndex.y >= obstacleMaxIndex.y)
            rightGridIndex = { obstacleMinIndex.x - 1, obstacleMaxIndex.y + 1 };
        else
            rightGridIndex = { obstacleMaxIndex.x + 1, obstacleMinIndex.y - 1 };
        forwardGridIndex = Vector2Add(rightGridIndex, {obstacleMinIndex.x - obstacleMaxIndex.x - 1, 0});
    }
    
    if (rightGridIndex.x < 0 || rightGridIndex.x >= gridSquares.at(0).size() ||
        rightGridIndex.y < 0 || rightGridIndex.y >= gridSquares.size() ||
        gridSquares[rightGridIndex.y][rightGridIndex.x]->occupied ||
        forwardGridIndex.x < 0 || forwardGridIndex.x >= gridSquares.at(0).size() ||
        forwardGridIndex.y < 0 || forwardGridIndex.y >= gridSquares.size() ||
        gridSquares[forwardGridIndex.y][forwardGridIndex.x]->occupied)
    {
        return { gridSquares[actorMinIndex.y][actorMinIndex.x]->worldPosMin };
    }
    
    return { gridSquares[rightGridIndex.y][rightGridIndex.x]->worldPosMin, gridSquares[forwardGridIndex.y][forwardGridIndex.x]->worldPosMin };
}

/**
 * Resolves local collisions by putting the obstacle into the grid system and using the pathfinding algorithm
 * for resolution.
 * @param actor 
 * @param obstacle 
 * @param startPos 
 * @param finishPos 
 * @return 
 */
std::vector<Vector3> NavigationGridSystem::PathfindAvoidLocalObstacle(entt::entity actor, BoundingBox obstacle, const Vector3& startPos, const Vector3& finishPos)
{
    // Get the grid indices for the bounding box
    Vector2 topLeftIndex;
    Vector2 bottomRightIndex;
    if (!WorldToGridSpace(obstacle.min, topLeftIndex) ||
        !WorldToGridSpace(obstacle.max, bottomRightIndex))
    {
        return {};
    }

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
            gridSquares[row][col]->debugColor = true;
        }
    }
    
    Vector2 minRange;
    Vector2 maxRange;
    int bounds = 50;
    if (registry->any_of<ControllableActor>(actor))
    {
        auto& controllableActor = registry->get<ControllableActor>(actor);
        bounds = controllableActor.pathfindingBounds;
    }

    if (!GetPathfindRange(actor, bounds, minRange, maxRange))
    {
        return {};
    }
    
    auto path = BFSPathfind(startPos, finishPos, minRange, maxRange);
    
    
    for (int row = min_row; row <= max_row; ++row)
    {
        for (int col = min_col; col <= max_col; ++col)
        {
            // Access grid square from the 2D array
            gridSquares[row][col]->occupied = false;
        }
    }
    return path;
}

/**
 * Generates a sequence of nodes that should be the "optimal" route from point A to point B.
 * Checks entire grid.
 * @return A vector of "nodes" to travel to in sequential order. Empty if path is invalid (OOB or no path available).
 */
std::vector<Vector3> NavigationGridSystem::AStarPathfind(const Vector3 &startPos, const Vector3 &finishPos)
{
    return AStarPathfind(startPos, finishPos, {0,0},
                       {static_cast<float>(gridSquares.at(0).size()), static_cast<float>(gridSquares.size())});
}

inline double heuristic(std::pair<int, int> a, std::pair<int, int> b) 
{
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

/**
 * Generates a sequence of nodes that should be the "optimal" route from point A to point B.
 * Checks path within a range. Use "GetPathfindRange" to calculate minRange/maxRange if needed.
 * @minRange The minimum grid index in the pathfinding range.
 * @maxRange The maximum grid index in the pathfinding range.
 * @return A vector of "nodes" to travel to in sequential order. Empty if path is invalid (OOB or no path available).
 */
std::vector<Vector3> NavigationGridSystem::AStarPathfind(const Vector3 &startPos,
                                                         const Vector3 &finishPos,
                                                         const Vector2 &minRange,
                                                         const Vector2 &maxRange)
{
    Vector2 startGridSquare = {0};
    Vector2 finishGridSquare = {0};
    if (!WorldToGridSpace(startPos, startGridSquare) || !WorldToGridSpace(finishPos, finishGridSquare)) return {};
    int startrow = startGridSquare.y;
    int startcol = startGridSquare.x;

    int finishrow = finishGridSquare.y;
    int finishcol = finishGridSquare.x;

    auto inside = [&](int row, int col) { return minRange.y <= row && row < maxRange.y && minRange.x <= col && col < maxRange.x; };

    std::vector<std::vector<bool>> visited(maxRange.y, std::vector<bool>(maxRange.x, false));
    std::vector<std::vector<std::pair<int, int>>> came_from(maxRange.y, std::vector<std::pair<int, int>>(maxRange.x, std::pair<int, int>(-1, -1)));
    std::vector<std::vector<double>> cost_so_far(maxRange.y, std::vector<double>(maxRange.x, 0.0));
    
    std::vector<std::pair<int, int>> directions = { {1,0}, {0,1}, {-1,0}, {0,-1}, {1,1}, {-1,1}, {-1,-1}, {1,-1} };
    PriorityQueue<std::pair<int,int>, double> frontier;

    frontier.put({startrow, startcol}, 0);
    visited[startrow][startcol] = true;

    bool pathFound = false;

    while (!frontier.empty())
    {
        auto current = frontier.get();

        if (current.first == finishrow && current.second == finishcol)
        {
            pathFound = true;
            break;
        }

        for (const auto& dir : directions)
        {
            int next_row = current.first + dir.first;
            int next_col = current.second + dir.second;
            
            auto current_cost = gridSquares[current.first][current.second]->pathfindingCost;
            auto next_cost = gridSquares[next_row][next_col]->pathfindingCost;
            double new_cost = current_cost + next_cost;
            
            if (inside(next_row, next_col) &&
                !visited[next_row][next_col] &&
                !gridSquares[next_row][next_col]->occupied || 
                new_cost < cost_so_far[next_row][next_col])
            {
                cost_so_far[next_row][next_col] = new_cost;
                double priority = new_cost + heuristic({next_row, next_col}, {finishrow, finishcol});
                frontier.put({next_row, next_col}, priority);
                came_from[next_row][next_col] = current;
                visited[next_row][next_col] = true;
            }
        }
    }

    if (!pathFound)
    {
        return {};
    }

    // Trace path back from finish to start, skip nodes if they are the same direction as the previous
    std::vector<Vector3> path;
    std::pair<int, int> current = {finishrow, finishcol};
    std::pair<int, int> previous;
    std::pair<int, int> currentDir = {0,0};

    path.push_back(gridSquares[current.first][current.second]->worldPosMin);
    while (current.first != startrow || current.second != startcol)
    {
        previous = current;
        current = came_from[current.first][current.second];
        for (const auto& dir : directions)
        {
            int row = previous.first + dir.first;
            int col = previous.second + dir.second;
            if (row == current.first && col == current.second) // Found the direction
            {
                if (currentDir.first == 0 && currentDir.second == 0)
                {
                    currentDir = dir;
                    break;
                }
                if (dir != currentDir)
                {
                    currentDir = dir;
                    path.push_back(gridSquares[previous.first][previous.second]->worldPosMin);
                    path.push_back(gridSquares[current.first][current.second]->worldPosMin);
                }
                break;
            }
        }
    }
    path.push_back(gridSquares[current.first][current.second]->worldPosMin);
    std::reverse(path.begin(), path.end());

    return path;
}

/**
 * Generates a sequence of nodes that should be the "optimal" route from point A to point B.
 * Checks entire grid.
 * @return A vector of "nodes" to travel to in sequential order. Empty if path is invalid (OOB or no path available).
 */
std::vector<Vector3> NavigationGridSystem::BFSPathfind(const Vector3& startPos, const Vector3& finishPos)
{
    return BFSPathfind(startPos, finishPos, {0,0},
                    {static_cast<float>(gridSquares.at(0).size()), static_cast<float>(gridSquares.size())});
}

/**
 * Generates a sequence of nodes that should be the "optimal" route from point A to point B.
 * Checks path within a range. Use "GetPathfindRange" to calculate minRange/maxRange if needed.
 * @minRange The minimum grid index in the pathfinding range.
 * @maxRange The maximum grid index in the pathfinding range.
 * @return A vector of "nodes" to travel to in sequential order. Empty if path is invalid (OOB or no path available).
 */
std::vector<Vector3> NavigationGridSystem::BFSPathfind(const Vector3& startPos, const Vector3& finishPos, const Vector2& minRange, const Vector2& maxRange)
{
    Vector2 startGridSquare = {0};
    Vector2 finishGridSquare = {0};
    if (!WorldToGridSpace(startPos, startGridSquare) || !WorldToGridSpace(finishPos, finishGridSquare)) return {};
    int startrow = startGridSquare.y;
    int startcol = startGridSquare.x;

    int finishrow = finishGridSquare.y;
    int finishcol = finishGridSquare.x;

    auto inside = [&](int row, int col) { return minRange.y <= row && row < maxRange.y && minRange.x <= col && col < maxRange.x; };

    std::vector<std::vector<bool>> visited(maxRange.y, std::vector<bool>(maxRange.x, false));
    std::vector<std::vector<std::pair<int, int>>> came_from(maxRange.y, std::vector<std::pair<int, int>>(maxRange.x, std::pair<int, int>(-1, -1)));

    std::vector<std::pair<int, int>> directions = { {1,0}, {0,1}, {-1,0}, {0,-1}, {1,1}, {-1,1}, {-1,-1}, {1,-1} };
    std::queue<std::pair<int,int>> frontier;

    frontier.emplace(startrow, startcol);
    visited[startrow][startcol] = true;

    bool pathFound = false;

    while (!frontier.empty())
    {
        auto current = frontier.front();
        frontier.pop();

        if (current.first == finishrow && current.second == finishcol) 
        {
            pathFound = true;
            break;
        }

        for (const auto& dir : directions)
        {
            int next_row = current.first + dir.first;
            int next_col = current.second + dir.second;

            if (inside(next_row, next_col) && 
            !visited[next_row][next_col] && 
            !gridSquares[next_row][next_col]->occupied)
            {
                frontier.emplace(next_row, next_col);
                visited[next_row][next_col] = true;
                came_from[next_row][next_col] = current;
            }
        }
    }
    
    if (!pathFound) 
    {
        return {}; 
    }

    // Trace path back from finish to start, skip nodes if they are the same direction as the previous
    std::vector<Vector3> path;
    std::pair<int, int> current = {finishrow, finishcol};
    std::pair<int, int> previous;
    std::pair<int, int> currentDir = {0,0};
    
    path.push_back(gridSquares[current.first][current.second]->worldPosMin);
    while (current.first != startrow || current.second != startcol)
    {
        previous = current;
        current = came_from[current.first][current.second];
        for (const auto& dir : directions)
        {
            int row = previous.first + dir.first;
            int col = previous.second + dir.second;
            if (row == current.first && col == current.second) // Found the direction
            {
                if (currentDir.first == 0 && currentDir.second == 0)
                {
                    currentDir = dir;
                    break;
                }
                if (dir != currentDir)
                {
                    currentDir = dir;
                    path.push_back(gridSquares[previous.first][previous.second]->worldPosMin);
                    path.push_back(gridSquares[current.first][current.second]->worldPosMin);
                }
                break;
            }
        }
    }
    path.push_back(gridSquares[current.first][current.second]->worldPosMin);
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
    
    const auto& view = registry->view<Collideable>();
    for (const auto& ent : view)
    {
        const auto& bb = view.get<Collideable>(ent);
        if (bb.collisionLayer != CollisionLayer::BUILDING) continue;

        // Get the grid indices for the bounding box
        Vector2 topLeftIndex;
        Vector2 bottomRightIndex;
        if (!WorldToGridSpace(bb.worldBoundingBox.min, topLeftIndex) ||
        !WorldToGridSpace(bb.worldBoundingBox.max, bottomRightIndex))
        {
            continue;
        }

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

NavigationGridSystem::NavigationGridSystem(entt::registry* _registry) :
    BaseSystem<NavigationGridSquare>(_registry)
{
}

} // sage