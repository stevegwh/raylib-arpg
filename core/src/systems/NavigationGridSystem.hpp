//
// Created by Steve Wheeler on 25/02/2024.
//

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
    static bool checkInside(int row, int col, Vector2 minRange, Vector2 maxRange);
	bool checkExtents(int row, int col, Vector2 extents) const;
  public:
    float spacing{};
    int slices{};

    explicit NavigationGridSystem(entt::registry* _registry);
    void Init(int _slices, float _spacing);
    void PopulateGrid() const;
    bool GetPathfindRange(const entt::entity& actorId, int bounds, Vector2& minRange, Vector2& maxRange) const;
    bool WorldToGridSpace(Vector3 worldPos, Vector2& out) const;
    bool WorldToGridSpace(Vector3 worldPos, Vector2& out, const Vector2& _minRange, const Vector2& _maxRange) const;
    [[nodiscard]] std::vector<Vector3>
    AStarPathfind(const entt::entity &entity, const Vector3 &startPos,
                  const Vector3 &finishPos,
                  AStarHeuristic heuristicType = AStarHeuristic::DEFAULT);
    [[nodiscard]] Vector2 FindNextBestLocation(entt::entity entity, Vector2 target) const;
    [[nodiscard]] entt::entity CastRay(int currentRow, int currentCol, Vector2 direction, int distance) const;
    [[nodiscard]] Vector2 FindNextBestLocation(Vector2 target, Vector2 minRange, Vector2 maxRange, Vector2 extents) const;
    [[nodiscard]] std::vector<Vector3> AStarPathfind(const entt::entity& entity, const Vector3& startPos, const Vector3& finishPos, const Vector2& minRange, const Vector2& maxRange, AStarHeuristic heuristicType = AStarHeuristic::DEFAULT);
    [[nodiscard]] std::vector<Vector3> ResolveLocalObstacle(entt::entity actor, BoundingBox obstacle, Vector3 currentDir) const;
    [[nodiscard]] std::vector<Vector3> PathfindAvoidLocalObstacle(entt::entity actor, const BoundingBox& obstacle, const Vector3& startPos, const Vector3& finishPos);
    [[nodiscard]] std::vector<Vector3> BFSPathfind(const Vector3& startPos, const Vector3& finishPos);
    [[nodiscard]] std::vector<Vector3> BFSPathfind(const Vector3& startPos, const Vector3& finishPos, const Vector2& minRange, const Vector2& maxRange);
    const std::vector<std::vector<NavigationGridSquare*>>& GetGridSquares();
    void DrawDebugPathfinding(const Vector2 &minRange, const Vector2 &maxRange) const;

    void MarkSquareOccupied(const BoundingBox& occupant, bool occupied, entt::entity occupantEntity = entt::null) const;
    bool CheckSingleSquareOccupied(Vector3 worldPos) const;
    bool CheckSingleSquareOccupied(Vector2 position) const;
    bool CheckSquareAreaOccupied(Vector3 worldPos, const BoundingBox& bb) const;
    bool CheckSquareAreaOccupied(int row, int col, const BoundingBox& bb) const;
	entt::entity CheckSingleSquareOccupant(Vector3 worldPos) const;
	entt::entity CheckSingleSquareOccupant(Vector2 position) const;
    entt::entity CheckSquareAreaOccupant(Vector3 worldPos, const BoundingBox& bb) const;
    entt::entity CheckSquareAreaOccupant(int row, int col, const BoundingBox& bb) const;
    bool CompareSquareAreaOccupant(entt::entity entity, const BoundingBox& bb) const;
    bool CompareSingleSquareOccupant(entt::entity entity, const BoundingBox& bb) const;

    void DrawDebug() const;
};

} // sage
