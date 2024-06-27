#include "NavigationGridSystem.hpp"
#include "components/ControllableActor.hpp"
#include "components/Transform.hpp"
#include "../utils/PriorityQueue.hpp"

#include <queue>
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

inline double heuristic(GridSquare a, GridSquare b)
{
    return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

inline double heuristic_favourRight(GridSquare a, GridSquare b, const Vector3& currentDir)
{
    double dx = std::abs(a.row - b.row);
    double dy = std::abs(a.col - b.col);
    double diagonal_distance = dx + dy;

    int currentX = std::round(currentDir.x);
    int currentZ = std::round(currentDir.z);

    if (currentZ > 0)
    {
        if (a.col < b.col)
        {
            diagonal_distance += 10.0;
        }
    } else if (currentZ < 0)
    {
        if (a.col > b.col)
        {
            diagonal_distance += 10.0;
        }
    } else if (currentX > 0)
    {
        if (a.row > b.row)
        {
            diagonal_distance += 10.0;
        }
    } else if (currentX < 0)
    {
        if (a.row < b.row)
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

    return checkExtents(gridPos, extents);
}

bool NavigationGridSystem::CheckSquareAreaOccupied(GridSquare square, const BoundingBox& bb) const
{
	GridSquare extents;
    {
        GridSquare bb_min;
        WorldToGridSpace(bb.min, bb_min);
        WorldToGridSpace(bb.max, extents);
        extents.row -= bb_min.row;
        extents.col -= bb_min.col;
    }
    return checkExtents(square, extents);
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
    return CheckSquareAreaOccupant(gridPos, bb);
}

entt::entity NavigationGridSystem::CheckSquareAreaOccupant(GridSquare square, const BoundingBox& bb) const
{
	GridSquare extents;
    {
        GridSquare bb_min;
        WorldToGridSpace(bb.min, bb_min);
        WorldToGridSpace(bb.max, extents);
        extents.row -= bb_min.row;
        extents.col -= bb_min.col;
    }

	if (!checkExtents(square, extents))
	{
		return entt::null;
	}

	if (gridSquares[square.row - extents.row][square.col - extents.col]->occupied)
	{
		return gridSquares[square.row - extents.row][square.col - extents.col]->occupant;
	}
	if (gridSquares[square.row + extents.row][square.col + extents.col]->occupied)
	{
		return gridSquares[square.row + extents.row][square.col + extents.col]->occupant;
	}
    if (gridSquares[square.row - extents.row][square.col + extents.col]->occupied)
	{
		return gridSquares[square.row - extents.row][square.col + extents.col]->occupant;
	}
    if (gridSquares[square.row + extents.row][square.col - extents.col]->occupied)
	{
		return gridSquares[square.row + extents.row][square.col - extents.col]->occupant;
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

        if (!checkInside(extents, { 0,0 }, 
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
    return checkInside(gridPos, {0,0}, maxRange);
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

std::vector<Vector3> NavigationGridSystem::tracebackPath(const std::vector<std::vector<GridSquare>>& came_from,
                                                         const GridSquare& start,
                                                         const GridSquare& finish) const
{
    std::vector<Vector3> path;
    GridSquare current = {finish.row, finish.col};
    GridSquare previous;
    std::pair<int, int> currentDir = {0,0};

    path.push_back(gridSquares[current.row][current.col]->worldPosMin);
    while (current.row != start.row || current.col != start.col)
    {
        previous = current;
        current = came_from[current.row][current.col];
        for (const auto& dir : directions)
        {
            int row = previous.row + dir.first;
            int col = previous.col + dir.second;
            if (row == current.row && col == current.col)
            {
                if (currentDir.first == 0 && currentDir.second == 0)
                {
                    currentDir = dir;
                    break;
                }
                if (dir != currentDir)
                {
                    currentDir = dir;
                    path.push_back(gridSquares[previous.row][previous.col]->worldPosMin);
                    path.push_back(gridSquares[current.row][current.col]->worldPosMin);
                }
                break;
            }
        }
    }
    path.push_back(gridSquares[current.row][current.col]->worldPosMin);
    std::ranges::reverse(path);
    return path;
}

bool NavigationGridSystem::checkInside(GridSquare square, GridSquare minRange, GridSquare maxRange)
{
	return minRange.row <= square.row && square.row < maxRange.row && minRange.col <= square.col && square.col < maxRange.col;
}

bool NavigationGridSystem::checkExtents(GridSquare square, GridSquare extents) const
{
	GridSquare min = {square.row - extents.row, square.col - extents.col};
	GridSquare max = {square.row + extents.row, square.col + extents.col};

	GridSquare minRange = {0, 0};
	GridSquare maxRange = {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())};

	return checkInside(min, minRange, maxRange) && 
        checkInside(max, minRange, maxRange) &&
        !gridSquares[square.row][square.col]->occupied &&
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
		GridSquare square = { currentRow + (dirRow * i), currentCol + (dirCol * i) };

        if (!checkInside(square, {0,0}, 
            {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())}))
	    {
		    continue;
	    }

        const auto cell = gridSquares[square.row][square.col];
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
    std::queue<GridSquare> frontier;
    frontier.emplace(target);

    GridSquare out{};
    bool foundValidSquare = false;
    while (!frontier.empty())
    {
        auto current = frontier.front();
        frontier.pop();
        for (const auto& dir : directions)
        {
			GridSquare next = { current.row + dir.first, current.col + dir.second };
            
            if (!checkInside(next, minRange, maxRange)) continue;
            
            if (!checkExtents(next, extents))
            {
                if (!visited[next.row][next.col])
                {
                    frontier.emplace(next);
                    visited[next.row][next.col] = true;
                }
            }
            else
            {
                out = next;
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
    
    if (!checkExtents(finishGridSquare, extents))
    {
        finishGridSquare = FindNextBestLocation(finishGridSquare, minRange, maxRange, extents);

    }

    std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
    std::vector<std::vector<GridSquare>> came_from(maxRange.row, std::vector<GridSquare>(maxRange.col, { -1, -1 }));
    std::vector<std::vector<double>> cost_so_far(maxRange.row, std::vector<double>(maxRange.col, 0.0));
    
    PriorityQueue<GridSquare, double> frontier;

    frontier.put(startGridSquare, 0);
    visited[startGridSquare.row][startGridSquare.col] = true;

    bool pathFound = false;

    while (!frontier.empty())
    {
        auto current = frontier.get();

        if (current.row == finishGridSquare.row && current.col == finishGridSquare.col)
        {
            pathFound = true;
            break;
        }

        for (const auto& dir : directions)
        {
            GridSquare next = { current.row + dir.first, current.col + dir.second };
            
            auto current_cost = gridSquares[current.row][current.col]->pathfindingCost;
            auto next_cost = gridSquares[next.row][next.col]->pathfindingCost;
            double new_cost = current_cost + next_cost;

            if (checkInside(next, minRange, maxRange) &&
                checkExtents(next, extents) &&
                !visited[next.row][next.col] &&
                !gridSquares.at(next.row).at(next.col)->occupied || 
                new_cost < cost_so_far[next.row][next.col])
            {
                cost_so_far[next.row][next.col] = new_cost;
                double heuristic_cost = 0;
                if (heuristicType == AStarHeuristic::DEFAULT)
                {
                    heuristic_cost = heuristic(next, finishGridSquare);
                }
                else if (heuristicType == AStarHeuristic::FAVOUR_RIGHT)
                {
                    auto& currentDir = registry->get<Transform>(entity).direction;
                    heuristic_cost = heuristic_favourRight(next, finishGridSquare, currentDir);
                }
                double priority = new_cost + heuristic_cost;
                frontier.put(next, priority);
                came_from[next.row][next.col] = current;
                visited[next.row][next.col] = true;
            }
        }
    }

    if (!pathFound)
    {
        return {};
    }
    
    return tracebackPath(came_from, startGridSquare, finishGridSquare);
}

//std::vector<Vector3> NavigationGridSystem::BFSPathfind(const Vector3& startPos, const Vector3& finishPos)
//{
//    return BFSPathfind(startPos, finishPos, {0,0},
//                    {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())});
//}
//
//std::vector<Vector3> NavigationGridSystem::BFSPathfind(const Vector3& startPos, const Vector3& finishPos, const GridSquare& minRange, const GridSquare& maxRange)
//{
//    GridSquare startGridSquare = {0};
//    GridSquare finishGridSquare = {0};
//    if (!WorldToGridSpace(startPos, startGridSquare) || !WorldToGridSpace(finishPos, finishGridSquare)) return {};
//    int startrow = startGridSquare.row;
//    int startcol = startGridSquare.col;
//
//    int finishrow = finishGridSquare.row;
//    int finishcol = finishGridSquare.col;
//
//    auto inside = [&](int row, int col) { return minRange.row <= row && row < maxRange.row && minRange.col <= col && col < maxRange.col; };
//
//    std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
//    std::vector<std::vector<std::pair<int, int>>> came_from(maxRange.row, std::vector<std::pair<int, int>>(maxRange.col, std::pair<int, int>(-1, -1)));
//
//    std::queue<std::pair<int,int>> frontier;
//
//    frontier.emplace(startrow, startcol);
//    visited[startrow][startcol] = true;
//
//    bool pathFound = false;
//
//    while (!frontier.empty())
//    {
//        auto current = frontier.front();
//        frontier.pop();
//
//        if (current.first == finishrow && current.second == finishcol) 
//        {
//            pathFound = true;
//            break;
//        }
//
//        for (const auto& dir : directions)
//        {
//            int next_row = current.first + dir.first;
//            int next_col = current.second + dir.second;
//
//            if (inside(next_row, next_col) && 
//            !visited[next_row][next_col] && 
//            !gridSquares[next_row][next_col]->occupied)
//            {
//                frontier.emplace(next_row, next_col);
//                visited[next_row][next_col] = true;
//                came_from[next_row][next_col] = current;
//            }
//        }
//    }
//    
//    if (!pathFound) 
//    {
//        return {}; 
//    }
//    
//    return tracebackPath(came_from, {startrow, startcol}, {finishrow, finishcol});
//}

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
