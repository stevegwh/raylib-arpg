#include "NavigationGridSystem.hpp"
#include "NavigationGridSystem.hpp"

#include "NavigationGridSystem.hpp"
#include "components/ControllableActor.hpp"
#include "components/Transform.hpp"
#include "../utils/PriorityQueue.hpp"

#include <queue>
#include <unordered_map>
#include <utility>
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

inline double heuristic(std::pair<int, int> a, std::pair<int, int> b)
{
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

inline double heuristic_favourRight(std::pair<int, int> a, std::pair<int, int> b, const Vector3& currentDir)
{
    double dx = std::abs(a.first - b.first);
    double dy = std::abs(a.second - b.second);
    double diagonal_distance = dx + dy;

    int currentX = std::round(currentDir.x);
    int currentZ = std::round(currentDir.z);

    if (currentZ > 0)
    {
        if (a.second < b.second)
        {
            diagonal_distance += 10.0;
        }
    } else if (currentZ < 0)
    {
        if (a.second > b.second)
        {
            diagonal_distance += 10.0;
        }
    } else if (currentX > 0)
    {
        if (a.first > b.first)
        {
            diagonal_distance += 10.0;
        }
    } else if (currentX < 0)
    {
        if (a.first < b.first)
        {
            diagonal_distance += 10.0;
        }
    }

    return diagonal_distance;
}

void NavigationGridSystem::Init(int _slices, float _spacing)
{
    slices = _slices;
    spacing = _spacing;
    
    int halfSlices = slices / 2;

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

            entt::entity id = registry->create();

            GridSquare gridSquareIndex = { i + halfSlices, j + halfSlices };
            auto& gridSquare = registry->emplace<NavigationGridSquare>(
                id, gridSquareIndex,v1,v3,calculateGridsquareCentre(v1, v3));
        	gridSquares[j + halfSlices][i + halfSlices] = &gridSquare;
        }
    }
}

void NavigationGridSystem::DrawDebugPathfinding(const GridSquare& minRange, const GridSquare& maxRange) const
{
    return;
    for (int i = 0; i  < gridSquares.size(); i++)
    {
        for (int j = 0; j < gridSquares.at(0).size(); j++)
        {
            gridSquares[i][j]->debugColor = false;
        }
    }
    for (int i = minRange.row; i  < maxRange.row; i++)
    {
        for (int j = minRange.col; j < maxRange.col; j++)
        {
            gridSquares[i][j]->debugColor = true;
        }
    }
}


void NavigationGridSystem::MarkSquareOccupied(const BoundingBox& occupant, bool occupied, entt::entity occupantEntity) const
{
    GridSquare topLeftIndex;
    GridSquare bottomRightIndex;
    if (!WorldToGridSpace(occupant.min, topLeftIndex) ||
        !WorldToGridSpace(occupant.max, bottomRightIndex))
    {
        return;
    }

    int min_col = std::min(topLeftIndex.col, bottomRightIndex.col);
    int max_col = std::max(topLeftIndex.col, bottomRightIndex.col);
    int min_row = std::min(topLeftIndex.row, bottomRightIndex.row);
    int max_row = std::max(topLeftIndex.row, bottomRightIndex.row);

    for (int row = min_row; row <= max_row; ++row)
    {
        for (int col = min_col; col <= max_col; ++col)
        {
            gridSquares[row][col]->occupied = occupied;
            gridSquares[row][col]->debugColor = occupied;
            if (occupied)
            {
	            gridSquares[row][col]->occupant = occupantEntity;
            }
            else
            {
	            gridSquares[row][col]->occupant = entt::null;
            }
        }
    }
}

bool NavigationGridSystem::CheckSingleSquareOccupied(Vector3 worldPos) const
{
    GridSquare squareIndex;
    if (!WorldToGridSpace(worldPos, squareIndex))
    {
	    return false;
    }
    return CheckSingleSquareOccupied(squareIndex);
}

bool NavigationGridSystem::CheckSingleSquareOccupied(GridSquare position) const
{
    return gridSquares[position.row][position.col]->occupied;
}

bool NavigationGridSystem::CheckSquareAreaOccupied(Vector3 worldPos, const BoundingBox& bb) const
{
	GridSquare extents, gridPos;
    {
        GridSquare bb_min;

        if (!WorldToGridSpace(bb.min, bb_min) ||
        !WorldToGridSpace(bb.max, extents) ||
        !WorldToGridSpace(worldPos, gridPos))
        {
			return false;	        
        }
        extents.row -= bb_min.row;
        extents.col -= bb_min.col;
    }

    return checkExtents(gridPos.row, gridPos.col, extents);
}

bool NavigationGridSystem::CheckSquareAreaOccupied(int row, int col, const BoundingBox& bb) const
{
	GridSquare extents;
    {
        GridSquare bb_min;
        WorldToGridSpace(bb.min, bb_min);
        WorldToGridSpace(bb.max, extents);
        extents.row -= bb_min.row;
        extents.col -= bb_min.col;
    }
    return checkExtents(row, col, extents);
}

entt::entity NavigationGridSystem::CheckSingleSquareOccupant(Vector3 worldPos) const
{
	GridSquare squareIndex;
    if (!WorldToGridSpace(worldPos, squareIndex))
    {
	    return entt::null;
    }
    return CheckSingleSquareOccupant(squareIndex);
}

entt::entity NavigationGridSystem::CheckSingleSquareOccupant(GridSquare position) const
{
    return gridSquares[position.row][position.col]->occupant;
}

entt::entity NavigationGridSystem::CheckSquareAreaOccupant(Vector3 worldPos, const BoundingBox& bb) const
{
    GridSquare gridPos;
    {
	    if (!WorldToGridSpace(worldPos, gridPos))
	    {
		    return entt::null;
	    }
    }
    return CheckSquareAreaOccupant(gridPos.row, gridPos.col, bb);
}

entt::entity NavigationGridSystem::CheckSquareAreaOccupant(int row, int col, const BoundingBox& bb) const
{
	GridSquare extents;
    {
        GridSquare bb_min;
        WorldToGridSpace(bb.min, bb_min);
        WorldToGridSpace(bb.max, extents);
        extents.row -= bb_min.row;
        extents.col -= bb_min.col;
    }

	if (!checkExtents(row, col, extents))
	{
		return entt::null;
	}

	if (gridSquares[row - extents.row][col - extents.col]->occupied)
	{
		return gridSquares[row - extents.row][col - extents.col]->occupant;
	}
	if (gridSquares[row + extents.row][col + extents.col]->occupied)
	{
		return gridSquares[row + extents.row][col + extents.col]->occupant;
	}
    if (gridSquares[row - extents.row][col + extents.col]->occupied)
	{
		return gridSquares[row - extents.row][col + extents.col]->occupant;
	}
    if (gridSquares[row + extents.row][col - extents.col]->occupied)
	{
		return gridSquares[row + extents.row][col - extents.col]->occupant;
	}
    return entt::null;
}

bool NavigationGridSystem::CompareSquareAreaOccupant(entt::entity entity, const BoundingBox& bb) const
{
    return false;
}

bool NavigationGridSystem::CompareSingleSquareOccupant(entt::entity entity, const BoundingBox& bb) const
{
    return false;
}

bool NavigationGridSystem::getExtents(entt::entity entity, GridSquare& extents) const
{
    {
        GridSquare bb_min;
        auto& bb = registry->get<Collideable>(entity).localBoundingBox;
        if (!WorldToGridSpace(bb.min, bb_min) || !WorldToGridSpace(bb.max, extents))
        {
	        return false;
        }
        extents.row -= bb_min.row;
        extents.col -= bb_min.col;

        if (!checkInside(extents.row, extents.col, { 0,0 }, 
            {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())}))
        {
	        return false;
        }
    }
    return true;
}

bool NavigationGridSystem::GetPathfindRange(const entt::entity& actorId, int bounds, GridSquare& minRange, GridSquare& maxRange) const
{
    auto bb = registry->get<Collideable>(actorId).worldBoundingBox;
    Vector3 center = { (bb.min.x + bb.max.x) / 2.0f, (bb.min.y + bb.max.y) / 2.0f, (bb.min.z + bb.max.z) / 2.0f };

    Vector3 topLeft = { center.x - bounds * spacing, center.y, center.z - bounds * spacing };
    Vector3 bottomRight = { center.x + bounds * spacing, center.y, center.z + bounds * spacing };

    GridSquare topLeftIndex;
    GridSquare bottomRightIndex;

    bool topLeftValid = WorldToGridSpace(topLeft, topLeftIndex);
    bool bottomRightValid = WorldToGridSpace(bottomRight, bottomRightIndex);
    
    if (!topLeftValid || !bottomRightValid) return false;

    topLeftIndex.col = std::max(topLeftIndex.col, 0);
    topLeftIndex.row = std::max(topLeftIndex.row, 0);
    bottomRightIndex.col = std::min(bottomRightIndex.col, static_cast<int>(gridSquares.at(0).size() - 1));
    bottomRightIndex.row = std::min(bottomRightIndex.row, static_cast<int>(gridSquares.size() - 1));

    minRange = { topLeftIndex.row, topLeftIndex.col };
    maxRange = { bottomRightIndex.row, bottomRightIndex.col };

    return true;
}

bool NavigationGridSystem::GridToWorldSpace(GridSquare gridPos, Vector3& out) const
{
    GridSquare maxRange = { static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size()) };
    out = gridSquares[gridPos.row][gridPos.col]->worldPosMin;
    return checkInside(gridPos.row, gridPos.col, {0,0}, maxRange);
}

bool NavigationGridSystem::WorldToGridSpace(Vector3 worldPos, GridSquare& out) const
{
    return WorldToGridSpace(worldPos, 
                            out, 
                            {0,0}, 
                            {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())});
}

bool NavigationGridSystem::WorldToGridSpace(Vector3 worldPos, GridSquare& out, const GridSquare& minRange, const GridSquare& maxRange) const
{
    int x = std::floor(worldPos.x / spacing) + (slices / 2);
    int y = std::floor(worldPos.z / spacing) + (slices / 2);
    out = {y, x};

    return out.row < maxRange.row && out.col < maxRange.col
    && out.col >= minRange.col && out.row >= minRange.row;
}

void NavigationGridSystem::DrawDebug() const
{
    for (const auto& gridSquareRow : gridSquares)
    {
        for (const auto& gridSquare : gridSquareRow)
        {
            if (!gridSquare->debugColor) continue;
            auto color = RED;
            DrawCubeWires(gridSquare->worldPosCentre,
                          gridSquare->debugBox.x,
                          gridSquare->debugBox.y,
                          gridSquare->debugBox.z,
                          color);
        }
    }
}

std::vector<Vector3> NavigationGridSystem::ResolveLocalObstacle(entt::entity actor, BoundingBox obstacle, Vector3 currentDir) const
{
    GridSquare actorMinIndex, actorMaxIndex, obstacleMinIndex, obstacleMaxIndex;
    auto& actorCollideable = registry->get<Collideable>(actor);
    if (!WorldToGridSpace(actorCollideable.worldBoundingBox.min, actorMinIndex) ||
        !WorldToGridSpace(actorCollideable.worldBoundingBox.max, actorMaxIndex) ||
        !WorldToGridSpace(obstacle.min, obstacleMinIndex) ||
        !WorldToGridSpace(obstacle.max, obstacleMaxIndex))
    {
        return { gridSquares[actorMinIndex.row][actorMinIndex.col]->worldPosMin };
    }
    
    GridSquare rightGridIndex{}, forwardGridIndex{};
    int currentX = std::round(currentDir.x);
    int currentZ = std::round(currentDir.z);
    
    if (currentZ > 0)
    {
        if (actorMaxIndex.col <= obstacleMinIndex.col)
            rightGridIndex = { obstacleMaxIndex.row, obstacleMinIndex.col - 1 };
        else
            rightGridIndex = { obstacleMinIndex.row, obstacleMaxIndex.col + 1 };
        forwardGridIndex = { rightGridIndex.row + 1, rightGridIndex.col };
    }
    else if (currentZ < 0)
    {
        if (actorMinIndex.col >= obstacleMaxIndex.col)
            rightGridIndex = { obstacleMinIndex.row - 1, obstacleMaxIndex.col + 1 };
        else
            rightGridIndex = { obstacleMaxIndex.row, obstacleMinIndex.col - 1 };
        forwardGridIndex = { rightGridIndex.row - 1, rightGridIndex.col };
    }

    if (currentX > 0)
    {
        if (actorMaxIndex.row <= obstacleMinIndex.row)
            rightGridIndex = { obstacleMinIndex.row - 1, obstacleMaxIndex.col + 1 };
        else
            rightGridIndex = { obstacleMaxIndex.row + 1, obstacleMinIndex.col };
        forwardGridIndex = { rightGridIndex.row, rightGridIndex.col + 1 };
    }
    else if (currentX < 0)
    {
        if (actorMinIndex.row >= obstacleMaxIndex.row)
            rightGridIndex = { obstacleMaxIndex.row + 1, obstacleMinIndex.col - 1 };
        else
            rightGridIndex = { obstacleMinIndex.row - 1, obstacleMaxIndex.col };
        forwardGridIndex = { rightGridIndex.row, rightGridIndex.col - 1 };
    }
    
    if (rightGridIndex.col < 0 || rightGridIndex.col >= gridSquares.at(0).size() ||
        rightGridIndex.row < 0 || rightGridIndex.row >= gridSquares.size() ||
        gridSquares[rightGridIndex.row][rightGridIndex.col]->occupied ||
        forwardGridIndex.col < 0 || forwardGridIndex.col >= gridSquares.at(0).size() ||
        forwardGridIndex.row < 0 || forwardGridIndex.row >= gridSquares.size() ||
        gridSquares[forwardGridIndex.row][forwardGridIndex.col]->occupied)
    {
        return { gridSquares[actorMinIndex.row][actorMinIndex.col]->worldPosMin };
    }
    
    return { gridSquares[rightGridIndex.row][rightGridIndex.col]->worldPosMin, gridSquares[forwardGridIndex.row][forwardGridIndex.col]->worldPosMin };
}

std::vector<Vector3> NavigationGridSystem::tracebackPath(const std::vector<std::vector<std::pair<int, int>>>& came_from,
                                                         const std::pair<int,int>& start,
                                                         const std::pair<int,int>& finish) const
{
    std::vector<Vector3> path;
    std::pair<int, int> current = {finish.first, finish.second};
    std::pair<int, int> previous;
    std::pair<int, int> currentDir = {0,0};

    path.push_back(gridSquares[current.first][current.second]->worldPosMin);
    while (current.first != start.first || current.second != start.second)
    {
        previous = current;
        current = came_from[current.first][current.second];
        for (const auto& dir : directions)
        {
            int row = previous.first + dir.first;
            int col = previous.second + dir.second;
            if (row == current.first && col == current.second)
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

bool NavigationGridSystem::checkInside(int row, int col, GridSquare minRange, GridSquare maxRange)
{
	return minRange.row <= row && row < maxRange.row && minRange.col <= col && col < maxRange.col;
}

bool NavigationGridSystem::checkExtents(int row, int col, GridSquare extents) const
{
	GridSquare min = {row - extents.row, col - extents.col};
	GridSquare max = {row + extents.row, col + extents.col};

	GridSquare minRange = {0, 0};
	GridSquare maxRange = {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())};

	return checkInside(min.row, min.col, minRange, maxRange) && 
        checkInside(max.row, max.col, minRange, maxRange) &&
        !gridSquares[row][col]->occupied &&
        !gridSquares[min.row][min.col]->occupied &&
        !gridSquares[min.row][max.col]->occupied &&
        !gridSquares[max.row][min.col]->occupied &&
        !gridSquares[max.row][max.col]->occupied;
}

NavigationGridSquare* NavigationGridSystem::CastRay(int currentRow, int currentCol, Vector2 direction, float distance) const
{
    int dist = std::round(distance);
    direction = Vector2Normalize(direction);
    int dirRow = std::round(direction.y);
    int dirCol = std::round(direction.x);

    for (int i = 0; i < dist; ++i)
    {
        int newRow = currentRow + (dirRow * i);
        int newCol = currentCol + (dirCol * i);

        if (!checkInside(newRow, newCol, {0,0}, 
            {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())}))
	    {
		    continue;
	    }

        const auto cell = gridSquares[newRow][newCol];
        cell->debugColor = true;

	    if (cell->occupant != entt::null)
	    {
            return cell;
	    }
    }
    return nullptr;
}


GridSquare NavigationGridSystem::FindNextBestLocation(entt::entity entity, GridSquare target) const
{

    GridSquare extents{};
    if (!getExtents(entity, extents))
    {
        return {};
    }
    GridSquare minRange, maxRange;
    int bounds = 50;
    if (!GetPathfindRange(entity, bounds, minRange, maxRange))
    {
        return {};
    }
    auto& destination = gridSquares[target.row][target.col];
    if (destination->occupied)
    {
        const auto& occupant = registry->get<Collideable>(destination->occupant);
        WorldToGridSpace(occupant.worldBoundingBox.max, target); // TODO: Fail?
    }

    return FindNextBestLocation(target, minRange, maxRange, extents);
}

GridSquare NavigationGridSystem::FindNextBestLocation(GridSquare target, GridSquare minRange, GridSquare maxRange, GridSquare extents) const
{

	std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
    std::queue<std::pair<int,int>> frontier;
    frontier.emplace(target.row, target.col);

    GridSquare out{};
    bool foundValidSquare = false;
    while (!frontier.empty())
    {
        auto current = frontier.front();
        frontier.pop();
        for (const auto& dir : directions)
        {
            int next_row = current.first + dir.first;
            int next_col = current.second + dir.second;
            
            if (!checkInside(next_row, next_col, minRange, maxRange)) continue;
            
            if (!checkExtents(next_row, next_col, extents))
            {
                if (!visited[next_row][next_col])
                {
                    frontier.emplace(next_row, next_col);
                    visited[next_row][next_col] = true;
                }
            }
            else
            {
                out.col = next_col;
                out.row = next_row;
                foundValidSquare = true;
                break;
            }
        }
        if (foundValidSquare)
            break;
    }
    
    return out;
}

std::vector<Vector3> NavigationGridSystem::AStarPathfind(const entt::entity& entity, const Vector3 &startPos, const Vector3 &finishPos, AStarHeuristic heuristicType)
{
    return AStarPathfind(entity,
                         startPos, 
                         finishPos, 
                         {0,0},
                         {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())},
                         heuristicType);
}

std::vector<Vector3> NavigationGridSystem::AStarPathfind(const entt::entity& entity,
                                                         const Vector3& startPos,
                                                         const Vector3& finishPos,
                                                         const GridSquare& minRange,
                                                         const GridSquare& maxRange,
                                                         AStarHeuristic heuristicType)
{
    GridSquare startGridSquare{};
    GridSquare finishGridSquare{};
    GridSquare extents{};
    
    if (!WorldToGridSpace(startPos, startGridSquare) || !WorldToGridSpace(finishPos, finishGridSquare) || !getExtents(entity, extents)) return {};
    
    if (!checkExtents(finishGridSquare.row, finishGridSquare.col, extents))
    {
        finishGridSquare = FindNextBestLocation(finishGridSquare, minRange, maxRange, extents);

    }
    
    int startrow = startGridSquare.row;
    int startcol = startGridSquare.col;

    int finishrow = finishGridSquare.row;
    int finishcol = finishGridSquare.col;
    
    std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
    std::vector<std::vector<std::pair<int, int>>> came_from(maxRange.row, std::vector<std::pair<int, int>>(maxRange.col, std::pair<int, int>(-1, -1)));
    std::vector<std::vector<double>> cost_so_far(maxRange.row, std::vector<double>(maxRange.col, 0.0));
    
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

            if (checkInside(next_row, next_col, minRange, maxRange) &&
                checkExtents(next_row, next_col, extents) &&
                !visited[next_row][next_col] &&
                !gridSquares.at(next_row).at(next_col)->occupied || 
                new_cost < cost_so_far[next_row][next_col])
            {
                cost_so_far[next_row][next_col] = new_cost;
                double heuristic_cost = 0;
                if (heuristicType == AStarHeuristic::DEFAULT)
                {
                    heuristic_cost = heuristic({next_row, next_col}, {finishrow, finishcol});
                }
                else if (heuristicType == AStarHeuristic::FAVOUR_RIGHT)
                {
                    auto& currentDir = registry->get<Transform>(entity).direction;
                    heuristic_cost = heuristic_favourRight({next_row, next_col}, {finishrow, finishcol}, currentDir);
                }
                double priority = new_cost + heuristic_cost;
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
    
    return tracebackPath(came_from, {startrow, startcol}, {finishrow, finishcol});
}

std::vector<Vector3> NavigationGridSystem::BFSPathfind(const Vector3& startPos, const Vector3& finishPos)
{
    return BFSPathfind(startPos, finishPos, {0,0},
                    {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())});
}

std::vector<Vector3> NavigationGridSystem::BFSPathfind(const Vector3& startPos, const Vector3& finishPos, const GridSquare& minRange, const GridSquare& maxRange)
{
    GridSquare startGridSquare = {0};
    GridSquare finishGridSquare = {0};
    if (!WorldToGridSpace(startPos, startGridSquare) || !WorldToGridSpace(finishPos, finishGridSquare)) return {};
    int startrow = startGridSquare.row;
    int startcol = startGridSquare.col;

    int finishrow = finishGridSquare.row;
    int finishcol = finishGridSquare.col;

    auto inside = [&](int row, int col) { return minRange.row <= row && row < maxRange.row && minRange.col <= col && col < maxRange.col; };

    std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
    std::vector<std::vector<std::pair<int, int>>> came_from(maxRange.row, std::vector<std::pair<int, int>>(maxRange.col, std::pair<int, int>(-1, -1)));

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
    
    return tracebackPath(came_from, {startrow, startcol}, {finishrow, finishcol});
}

void NavigationGridSystem::PopulateGrid() const
{
    for (auto& row : gridSquares)
    {
        for (auto& gridSquare : row)
        {
            gridSquare->occupied = false;
        }
    }
    
    const auto& view = registry->view<Collideable>();
    for (const auto& entity : view)
    {
        const auto& bb = view.get<Collideable>(entity);
        if (bb.collisionLayer != CollisionLayer::BUILDING) continue;
        MarkSquareOccupied(bb.worldBoundingBox, true, entity);
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
