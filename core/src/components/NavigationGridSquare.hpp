//
// Created by Steve Wheeler on 25/02/2024.
//

#pragma once

#include "raylib.h"
#include "raymath.h"
#include <entt/entt.hpp>

namespace sage
{
    struct GridSquare
    {
        int row;
        int col;

        bool operator>(const GridSquare& other) const
        {
            return std::tie(row, col) > std::tie(other.row, other.col);
        }

        bool operator<(const GridSquare& other) const
        {
            return std::tie(row, col) < std::tie(other.row, other.col);
        }

        bool operator==(const GridSquare& other) const
        {
            return std::tie(row, col) == std::tie(other.row, other.col);
        }

        bool operator!=(const GridSquare& other) const
        {
            return !(*this == other);
        }

        GridSquare operator-(const GridSquare& other) const
        {
            return {row - other.row, col - other.col};
        }

        void operator-=(const GridSquare& other)
        {
            row -= other.row;
            col -= other.col;
        }

        GridSquare operator+(const GridSquare& other) const
        {
            return {row + other.row, col + other.col};
        }

        void operator+=(const GridSquare& other)
        {
            row += other.row;
            col += other.col;
        }
    };

    struct NavigationGridSquare
    {
        int pathfindingCost = 1;
        bool drawDebug = false;
        Color debugColor = RED;
        GridSquare gridSquareIndex;
        Vector3 worldPosMin; // Top Left
        Vector3 worldPosMax; // Bottom Right
        Vector3 worldPosCentre;
        Vector3 debugBox;
        entt::entity occupant = entt::null;
        bool occupied = false;

        Vector3 terrainNormal = {0, 1, 0};
        // float terrainHeight = 0.0f;

        std::optional<float> terrainHeight;

        NavigationGridSquare(
            GridSquare _gridSquareIndex, Vector3 _worldPosMin, Vector3 _worldPosMax, Vector3 _worldPosCentre)
            : gridSquareIndex(_gridSquareIndex),
              worldPosMin(_worldPosMin),
              worldPosMax(_worldPosMax),
              worldPosCentre(_worldPosCentre),
              debugBox({fabsf(worldPosMax.x - worldPosMin.x), 0.1f, fabsf(worldPosMax.z - worldPosMin.z)})
        {
        }
    };
} // namespace sage
