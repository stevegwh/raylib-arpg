#pragma once

#include "BaseSystem.hpp"
#include "../components/NavigationGridSquare.hpp"
#include "CollisionSystem.hpp"

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
    std::vector<Vector3> tracebackPath(const std::vector<std::vector<std::pair<int, int>>>& currentPath,
                                       const std::pair<int,int>& start,
                                       const std::pair<int,int>& finish) const;
    static bool checkInside(int row, int col, GridSquare minRange, GridSquare maxRange);
	bool getExtents(entt::entity entity, GridSquare& extents) const;
	bool checkExtents(int row, int col, GridSquare extents) const;
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
    [[nodiscard]] std::vector<Vector3>
    AStarPathfind(const entt::entity &entity, const Vector3 &startPos,
                  const Vector3 &finishPos,
                  AStarHeuristic heuristicType = AStarHeuristic::DEFAULT);
    [[nodiscard]] GridSquare FindNextBestLocation(entt::entity entity, GridSquare target) const;
    [[nodiscard]] NavigationGridSquare* CastRay(int currentRow, int currentCol, Vector2 direction, float distance) const;
    [[nodiscard]] GridSquare FindNextBestLocation(GridSquare target, GridSquare minRange, GridSquare maxRange, GridSquare extents) const;
    [[nodiscard]] std::vector<Vector3> AStarPathfind(const entt::entity& entity, const Vector3& startPos, const Vector3& finishPos, const GridSquare& minRange, const GridSquare& maxRange, AStarHeuristic heuristicType = AStarHeuristic::DEFAULT);
    [[nodiscard]] std::vector<Vector3> ResolveLocalObstacle(entt::entity actor, BoundingBox obstacle, Vector3 currentDir) const;
    [[nodiscard]] std::vector<Vector3> BFSPathfind(const Vector3& startPos, const Vector3& finishPos);
    [[nodiscard]] std::vector<Vector3> BFSPathfind(const Vector3& startPos, const Vector3& finishPos, const GridSquare& minRange, const GridSquare& maxRange);
    const std::vector<std::vector<NavigationGridSquare*>>& GetGridSquares();
    void DrawDebugPathfinding(const GridSquare &minRange, const GridSquare &maxRange) const;

    void MarkSquareOccupied(const BoundingBox& occupant, bool occupied, entt::entity occupantEntity = entt::null) const;
    bool CheckSingleSquareOccupied(Vector3 worldPos) const;
    bool CheckSingleSquareOccupied(GridSquare position) const;
    bool CheckSquareAreaOccupied(Vector3 worldPos, const BoundingBox& bb) const;
    bool CheckSquareAreaOccupied(int row, int col, const BoundingBox& bb) const;
	entt::entity CheckSingleSquareOccupant(Vector3 worldPos) const;
	entt::entity CheckSingleSquareOccupant(GridSquare position) const;
    entt::entity CheckSquareAreaOccupant(Vector3 worldPos, const BoundingBox& bb) const;
    entt::entity CheckSquareAreaOccupant(int row, int col, const BoundingBox& bb) const;
    bool CompareSquareAreaOccupant(entt::entity entity, const BoundingBox& bb) const;
    bool CompareSingleSquareOccupant(entt::entity entity, const BoundingBox& bb) const;

    void DrawDebug() const;
};

} // sage
