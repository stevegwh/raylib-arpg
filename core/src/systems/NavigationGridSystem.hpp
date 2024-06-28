#pragma once

#include "BaseSystem.hpp"
#include "../components/NavigationGridSquare.hpp"
#include "CollisionSystem.hpp"
#include "../utils/PriorityQueue.hpp"

#include "entt/entt.hpp"

namespace sage
{

enum class AStarHeuristic
{
    DEFAULT,
    FAVOUR_RIGHT
};

class NavigationGridSystem : public BaseSystem<NavigationGridSquare>
{
    std::vector<std::pair<int, int>> directions = { {1,0}, {0,1}, {-1,0}, {0,-1}, {1,1}, {-1,1}, {-1,-1}, {1,-1} };
    std::vector<std::vector<NavigationGridSquare*>> gridSquares;
	std::vector<Vector3> tracebackPath(const std::vector<std::vector<GridSquare>>& came_from,
	                                                         const GridSquare& start,
	                                                         const GridSquare& finish) const;
    static bool checkInside(GridSquare square, GridSquare minRange, GridSquare maxRange);
	bool getExtents(entt::entity entity, GridSquare& extents) const;
	bool checkExtents(GridSquare square, GridSquare extents) const;
	bool getExtents(Vector3 worldPos, GridSquare& extents) const;
  public:
    float spacing{};
    int slices{};

    explicit NavigationGridSystem(entt::registry* _registry);
    void Init(int _slices, float _spacing);
    void PopulateGrid() const;
    bool GetPathfindRange(const entt::entity& actorId, int bounds, GridSquare& minRange, GridSquare& maxRange) const;
    bool GridToWorldSpace(GridSquare gridPos, Vector3& out) const;
    bool WorldToGridSpace(Vector3 worldPos, GridSquare& out) const;
    bool WorldToGridSpace(Vector3 worldPos, GridSquare& out, const GridSquare& _minRange, const GridSquare& _maxRange) const;
    [[nodiscard]] GridSquare FindNextBestLocation(entt::entity entity, GridSquare target) const;
    [[nodiscard]] GridSquare FindNextBestLocation(GridSquare currentPos, GridSquare target, GridSquare minRange, GridSquare maxRange, GridSquare extents) const;
    [[nodiscard]] NavigationGridSquare* CastRay(int currentRow, int currentCol, Vector2 direction, float distance) const;
	[[nodiscard]] std::vector<Vector3> AStarPathfind(const entt::entity &entity, const Vector3 &startPos,
                  const Vector3 &finishPos,
                  AStarHeuristic heuristicType = AStarHeuristic::DEFAULT);
    [[nodiscard]] std::vector<Vector3> AStarPathfind(const entt::entity& entity, const Vector3& startPos, const Vector3& finishPos, const GridSquare& minRange, const GridSquare& maxRange, AStarHeuristic heuristicType = AStarHeuristic::DEFAULT);
    const std::vector<std::vector<NavigationGridSquare*>>& GetGridSquares();
    void DrawDebugPathfinding(const GridSquare &minRange, const GridSquare &maxRange) const;

    void MarkSquareOccupied(const BoundingBox& occupant, bool occupied, entt::entity occupantEntity = entt::null) const;
    bool CheckSingleSquareOccupied(Vector3 worldPos) const;
    bool CheckSingleSquareOccupied(GridSquare position) const;
    bool CheckBoundingBoxAreaUnoccupied(Vector3 worldPos, const BoundingBox& bb) const;
    bool CheckBoundingBoxAreaUnoccupied(GridSquare square, const BoundingBox& bb) const;
	entt::entity CheckSingleSquareOccupant(Vector3 worldPos) const;
	entt::entity CheckSingleSquareOccupant(GridSquare position) const;
    entt::entity CheckSquareAreaOccupant(Vector3 worldPos, const BoundingBox& bb) const;
    entt::entity CheckSquareAreaOccupant(GridSquare square, const BoundingBox& bb) const;
    bool CompareSquareAreaOccupant(entt::entity entity, const BoundingBox& bb) const;
    bool CompareSingleSquareOccupant(entt::entity entity, const BoundingBox& bb) const;

    void DrawDebug() const;

    template<typename Heuristic>
	std::vector<Vector3> AStarPathfindHelper(const entt::entity& entity, const GridSquare& startGridSquare, const GridSquare& finishGridSquare, const GridSquare& minRange, const GridSquare& maxRange, const GridSquare& extents, Heuristic heuristic) {
	    std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
	    std::vector<std::vector<GridSquare>> came_from(maxRange.row, std::vector<GridSquare>(maxRange.col, { -1, -1 }));
	    std::vector<std::vector<double>> cost_so_far(maxRange.row, std::vector<double>(maxRange.col, 0.0));

	    PriorityQueue<GridSquare, double> frontier;
	    frontier.put(startGridSquare, 0);
	    visited[startGridSquare.row][startGridSquare.col] = true;

	    bool pathFound = false;

	    while (!frontier.empty()) {
	        auto current = frontier.get();

	        if (current.row == finishGridSquare.row && current.col == finishGridSquare.col) {
	            pathFound = true;
	            break;
	        }

	        for (const auto& dir : directions) {
	            GridSquare next = { current.row + dir.first, current.col + dir.second };

	            auto current_cost = gridSquares[current.row][current.col]->pathfindingCost;
	            auto next_cost = gridSquares[next.row][next.col]->pathfindingCost;
	            double new_cost = current_cost + next_cost;

	            if (checkInside(next, minRange, maxRange) &&
	                checkExtents(next, extents) &&
	                !visited[next.row][next.col] &&
	                !gridSquares.at(next.row).at(next.col)->occupied || 
	                new_cost < cost_so_far[next.row][next.col]) {
	                cost_so_far[next.row][next.col] = new_cost;
	                double priority = new_cost + heuristic(next, finishGridSquare);
	                frontier.put(next, priority);
	                came_from[next.row][next.col] = current;
	                visited[next.row][next.col] = true;
	            }
	        }
	    }

	    if (!pathFound) {
	        return {};
	    }

	    return tracebackPath(came_from, startGridSquare, finishGridSquare);
	}
};

} // sage
