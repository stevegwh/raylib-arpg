#include "NavigationGridSystem.hpp"

#include "CollisionSystem.hpp"
#include "components/ControllableActor.hpp"
#include "components/NavigationGridSquare.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include <Serializer.hpp>

#include <iostream>
#include <queue>

Vector3 calculateGridsquareCentre(Vector3 min, Vector3 max)
{
    Vector3 size = {0};

    size.x = fabsf(max.x - min.x);
    size.y = fabsf(max.y - min.y);
    size.z = fabsf(max.z - min.z);

    return {min.x + size.x / 2.0f, min.y + size.y / 2.0f, min.z + size.z / 2.0f};
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
                diagonal_distance += 1.0;
            }
        }
        else if (currentZ < 0)
        {
            if (a.col > b.col)
            {
                diagonal_distance += 1.0;
            }
        }
        else if (currentX > 0)
        {
            if (a.row > b.row)
            {
                diagonal_distance += 1.0;
            }
        }
        else if (currentX < 0)
        {
            if (a.row < b.row)
            {
                diagonal_distance += 1.0;
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
                Vector3 v1 = {static_cast<float>(i) * spacing, 0, static_cast<float>(j) * spacing};
                Vector3 v3 = {static_cast<float>(i + 1) * spacing, 1.0f, static_cast<float>(j + 1) * spacing};

                entt::entity id = registry->create();

                GridSquare gridSquareIndex = {i + halfSlices, j + halfSlices};
                auto& gridSquare = registry->emplace<NavigationGridSquare>(
                    id, gridSquareIndex, v1, v3, calculateGridsquareCentre(v1, v3));
                gridSquares[j + halfSlices][i + halfSlices] = &gridSquare;
            }
        }
    }

    void NavigationGridSystem::DrawDebugPathfinding(const GridSquare& minRange, const GridSquare& maxRange) const
    {
        return;
        for (int i = 0; i < gridSquares.size(); i++)
        {
            for (int j = 0; j < gridSquares.at(0).size(); j++)
            {
                gridSquares[i][j]->drawDebug = false;
            }
        }
        for (int i = minRange.row; i < maxRange.row; i++)
        {
            for (int j = minRange.col; j < maxRange.col; j++)
            {
                gridSquares[i][j]->drawDebug = true;
            }
        }
    }

    void NavigationGridSystem::MarkSquareAreaOccupiedIfSteep(const BoundingBox& occupant, bool occupied)
    {
        GridSquare topLeftIndex{};
        GridSquare bottomRightIndex{};
        if (!WorldToGridSpace(occupant.min, topLeftIndex) || !WorldToGridSpace(occupant.max, bottomRightIndex))
        {
            return;
        }

        int min_col = std::min(topLeftIndex.col, bottomRightIndex.col);
        int max_col = std::max(topLeftIndex.col, bottomRightIndex.col);
        int min_row = std::min(topLeftIndex.row, bottomRightIndex.row);
        int max_row = std::max(topLeftIndex.row, bottomRightIndex.row);
        Vector3 up = {0.0f, 1.0f, 0.0f};

        for (int row = min_row; row <= max_row; ++row)
        {
            for (int col = min_col; col <= max_col; ++col)
            {
                auto normal = gridSquares[row][col]->terrainNormal;
                // Calculate the angle between the normal and the up vector
                float dotProduct = normal.x * up.x + normal.y * up.y + normal.z * up.z;
                float angle = std::acos(dotProduct) * RAD2DEG; // Convert to degrees

                // If the angle is greater than the max slope angle, return a very high
                // cost
                if (angle > 45.0f)
                {
                    gridSquares[row][col]->occupied = occupied;
                    gridSquares[row][col]->drawDebug = occupied;
                }
            }
        }
    }

    void NavigationGridSystem::MarkSquareAreaOccupied(
        const BoundingBox& occupant, bool occupied, entt::entity occupantEntity) const
    {
        GridSquare topLeftIndex{};
        GridSquare bottomRightIndex{};
        if (!WorldToGridSpace(occupant.min, topLeftIndex) || !WorldToGridSpace(occupant.max, bottomRightIndex))
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
                gridSquares[row][col]->drawDebug = occupied;
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

    void NavigationGridSystem::MarkSquaresOccupied(const std::vector<GridSquare>& squares, bool occupied) const
    {
        for (const auto& square : squares)
        {
            gridSquares[square.row][square.col]->occupied = occupied;
        }
    }

    void NavigationGridSystem::MarkSquaresDebug(
        const std::vector<GridSquare>& squares, Color color, bool occupied) const
    {
        for (const auto& square : squares)
        {
            gridSquares[square.row][square.col]->drawDebug = occupied;
            if (occupied)
            {
                gridSquares[square.row][square.col]->debugColor = color;
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

    /**
     * Checks whether the bounding box fits at the given world position.
     * @param worldPos
     * @param bb
     * @return
     */
    bool NavigationGridSystem::CheckBoundingBoxAreaUnoccupied(Vector3 worldPos, const BoundingBox& bb) const
    {
        GridSquare gridPos;
        if (!WorldToGridSpace(worldPos, gridPos))
        {
            return false;
        }

        return CheckBoundingBoxAreaUnoccupied(gridPos, bb);
    }

    bool NavigationGridSystem::CheckBoundingBoxAreaUnoccupied(GridSquare square, const BoundingBox& bb) const
    {
        GridSquare extents;
        {
            GridSquare bb_min;
            if (!WorldToGridSpace(bb.min, bb_min) || !WorldToGridSpace(bb.max, extents))
            {
                return false;
            }

            extents -= bb_min;
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
            extents -= bb_min;
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

    float calculateTerrainCost(const Vector3& normal, float maxSlopeAngle)
    {
        // The up vector
        Vector3 up = {0.0f, 1.0f, 0.0f};

        // Calculate the angle between the normal and the up vector
        float dotProduct = normal.x * up.x + normal.y * up.y + normal.z * up.z;
        float angle = std::acos(dotProduct) * RAD2DEG; // Convert to degrees

        // If the angle is greater than the max slope angle, return a very high cost
        if (angle > maxSlopeAngle)
        {
            return std::numeric_limits<float>::max();
        }

        // Otherwise, calculate a cost based on the angle
        // This will return 1.0 for flat ground, and increase as the slope increases
        return 1.0f + (angle / maxSlopeAngle);
    }

    void NavigationGridSystem::calculateTerrainHeightAndNormals(const entt::entity& entity)
    {
        BoundingBox area = registry->get<Collideable>(entity).worldBoundingBox;
        GridSquare topLeftIndex{}, bottomRightIndex{};
        if (!WorldToGridSpace(area.min, topLeftIndex) || !WorldToGridSpace(area.max, bottomRightIndex))
        {
            return;
        }

        auto& renderable = registry->get<Renderable>(entity);
        auto& transform = registry->get<sgTransform>(entity);

        int min_col = std::max(0, std::min(topLeftIndex.col, bottomRightIndex.col));
        int max_col = std::min(
            static_cast<int>(gridSquares[0].size()) - 1, std::max(topLeftIndex.col, bottomRightIndex.col));
        int min_row = std::max(0, std::min(topLeftIndex.row, bottomRightIndex.row));
        int max_row =
            std::min(static_cast<int>(gridSquares.size()) - 1, std::max(topLeftIndex.row, bottomRightIndex.row));

        for (int row = min_row; row <= max_row; ++row)
        {
            for (int col = min_col; col <= max_col; ++col)
            {
                Vector3 gridCenter = {
                    (gridSquares[row][col]->worldPosMin.x + gridSquares[row][col]->worldPosMax.x) * 0.5f,
                    area.max.y + 1.0f, // Start slightly above the terrain
                    (gridSquares[row][col]->worldPosMin.z + gridSquares[row][col]->worldPosMax.z) * 0.5f};

                Ray ray = {gridCenter, {0, -1, 0}}; // Cast ray down

                RayCollision collision = renderable.GetModel()->GetRayMeshCollision(ray, 0, transform.GetMatrix());

                if (collision.hit)
                {
                    if (gridSquares[row][col]->terrainHeight < collision.point.y)
                    {
                        gridSquares[row][col]->terrainHeight = collision.point.y;
                        gridSquares[row][col]->terrainNormal = collision.normal;
                        // gridSquares[row][col]->pathfindingCost =
                        // calculateTerrainCost(collision.normal, 45.0f);
                    }
                }
            }
        }
    }

    void NavigationGridSystem::GenerateNormalMap(ImageSafe& image)
    {
        int slices = gridSquares.size();

        Image normalMap = GenImageColor(slices, slices, BLACK);
        std::cout << "Generating normal map..." << std::endl;
        for (int y = 0; y < slices; ++y)
        {
            for (int x = 0; x < slices; ++x)
            {
                auto normal = gridSquares[y][x]->terrainNormal;

                // Map the normal components from [-1, 1] to [0, 255]
                unsigned char r = static_cast<unsigned char>((normal.x + 1.0f) * 127.5f);
                unsigned char g = static_cast<unsigned char>((normal.y + 1.0f) * 127.5f);
                unsigned char b = static_cast<unsigned char>((normal.z + 1.0f) * 127.5f);

                Color pixelColor = {r, g, b, 255};
                ImageDrawPixel(&normalMap, x, y, pixelColor);
            }
        }
        image.SetImage(normalMap);
    }

    float NavigationGridSystem::getMaxHeight(float slices)
    {
        float max = 0;
        BoundingBox bb = {.min = {-slices, 0.1f, -slices}, .max = {slices, 0.1f, slices}};

        auto inside = [bb](float x, float z) {
            return x >= bb.min.x && x <= bb.max.x && z >= bb.min.z && z <= bb.max.z;
        };

        auto view = registry->view<Collideable, Renderable>();

        for (const auto& entity : view)
        {
            const auto& c = registry->get<Collideable>(entity);
            if (c.collisionLayer != CollisionLayer::FLOOR) continue;

            // Check if either min or max point of the bounding box is inside the defined
            // area
            if (inside(c.worldBoundingBox.min.x, c.worldBoundingBox.min.z) ||
                inside(c.worldBoundingBox.max.x, c.worldBoundingBox.max.z))
            {
                if (c.worldBoundingBox.max.y > max)
                {
                    max = c.worldBoundingBox.max.y;
                }
            }
        }

        return max;
    }

    void NavigationGridSystem::GenerateHeightMap(ImageSafe& image)
    {
        int slices = gridSquares.size();
        float maxHeight = getMaxHeight(slices); // TODO

        Image heightMap = GenImageColor(slices, slices, BLACK);
        std::cout << "Generating height map..." << std::endl;
        for (int y = 0; y < slices; ++y)
        {
            for (int x = 0; x < slices; ++x)
            {
                float height = gridSquares[y][x]->terrainHeight;

                unsigned char heightValue =
                    static_cast<unsigned char>(std::min((height / maxHeight) * 255.0f, 255.0f));

                Color pixelColor = {heightValue, heightValue, heightValue, 255};
                ImageDrawPixel(&heightMap, x, y, pixelColor);
            }
        }
        image.SetImage(heightMap);
    }

    void NavigationGridSystem::loadTerrainNormalMap(const ImageSafe& normalMap)
    {
        for (int j = 0; j < slices; ++j)
        {
            for (int i = 0; i < slices; ++i)
            {
                // Get the color of the pixel
                Color color = normalMap.GetColor(i, j);

                // Convert the color values back to the range [-1, 1]
                float normalX = (static_cast<float>(color.r) / 127.5f) - 1.0f;
                float normalY = (static_cast<float>(color.g) / 127.5f) - 1.0f;
                float normalZ = (static_cast<float>(color.b) / 127.5f) - 1.0f;

                // Create a vector from these components
                Vector3 normal = {normalX, normalY, normalZ};

                // Normalize the vector (in case of any precision loss)
                normal = Vector3Normalize(normal);

                // Assign the normal to the corresponding grid square
                if (i >= 0 && i < slices && j >= 0 && j < slices)
                {
                    gridSquares[j][i]->terrainNormal = normal;
                }
            }
        }

        std::cout << "Terrain normal map loaded and applied to grid." << std::endl;
    }

    void NavigationGridSystem::loadTerrainHeightMap(const ImageSafe& heightMap, float maxHeight)
    {

        int halfSlices = slices / 2;

        for (int j = 0; j < slices; ++j)
        {
            for (int i = 0; i < slices; ++i)
            {
                // Get the color of the pixel
                Color color = heightMap.GetColor(i, j);

                // The height is stored in the red channel, normalized to 0-255
                float normalizedHeight = static_cast<float>(color.r) / 255.0f;

                // Scale the height to the maxHeight
                float height = normalizedHeight * maxHeight;

                // Calculate the grid index
                int gridX = i;
                int gridY = j;

                // Assign the height to the corresponding grid square
                if (gridX >= 0 && gridX < slices && gridY >= 0 && gridY < slices)
                {
                    gridSquares[gridY][gridX]->terrainHeight = height;
                }
            }
        }

        std::cout << "Terrain height map loaded and applied to grid." << std::endl;
    }

    /**
     * Checks a position in the world for an occupant. If an occupant is found, the
     * extents of the occupant are returned.
     * @param worldPos The position in the world to check for an occupant.
     * @param extents The extents of the occupant.
     * @return Whether an occupant was found
     */
    bool NavigationGridSystem::getExtents(Vector3 worldPos, GridSquare& extents) const
    {
        GridSquare gridPos;
        if (!WorldToGridSpace(worldPos, gridPos))
        {
            return false;
        }
        auto entity = CheckSingleSquareOccupant(worldPos);
        if (entity == entt::null)
        {
            return false;
        }

        if (!getExtents(entity, extents))
        {
            return false;
        }

        return true;
    }

    /**
     * Takes an entity and returns the extents of the entity in grid space.
     * @param entity The entity to get the extents of.
     * @param extents The extents of the entity.
     * @return Whether the extents were successfully retrieved.
     */
    bool NavigationGridSystem::getExtents(entt::entity entity, GridSquare& extents) const
    {
        GridSquare bb_min;
        auto& bb = registry->get<Collideable>(entity).localBoundingBox;
        if (!WorldToGridSpace(bb.min, bb_min) || !WorldToGridSpace(bb.max, extents))
        {
            return false;
        }

        extents -= bb_min;

        if (!checkInside(
                extents,
                {0, 0},
                {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())}))
        {
            return false;
        }

        return true;
    }

    bool NavigationGridSystem::GetGridRange(
        Vector3 center, int bounds, GridSquare& minRange, GridSquare& maxRange) const
    {
        Vector3 topLeft = {center.x - bounds * spacing, center.y, center.z - bounds * spacing};
        Vector3 bottomRight = {center.x + bounds * spacing, center.y, center.z + bounds * spacing};

        GridSquare topLeftIndex;
        GridSquare bottomRightIndex;

        bool topLeftValid = WorldToGridSpace(topLeft, topLeftIndex);
        bool bottomRightValid = WorldToGridSpace(bottomRight, bottomRightIndex);

        if (!topLeftValid || !bottomRightValid) return false;

        topLeftIndex.col = std::max(topLeftIndex.col, 0);
        topLeftIndex.row = std::max(topLeftIndex.row, 0);
        bottomRightIndex.col = std::min(bottomRightIndex.col, static_cast<int>(gridSquares.at(0).size() - 1));
        bottomRightIndex.row = std::min(bottomRightIndex.row, static_cast<int>(gridSquares.size() - 1));

        minRange = {topLeftIndex.row, topLeftIndex.col};
        maxRange = {bottomRightIndex.row, bottomRightIndex.col};

        return true;
    }

    bool NavigationGridSystem::GetGridRange(
        BoundingBox bb, int bounds, GridSquare& minRange, GridSquare& maxRange) const
    {
        Vector3 center = {
            (bb.min.x + bb.max.x) / 2.0f, (bb.min.y + bb.max.y) / 2.0f, (bb.min.z + bb.max.z) / 2.0f};
        return GetGridRange(center, bounds, minRange, maxRange);
    }

    bool NavigationGridSystem::GetPathfindRange(
        const entt::entity& actorId, int bounds, GridSquare& minRange, GridSquare& maxRange) const
    {
        auto bb = registry->get<Collideable>(actorId).worldBoundingBox;
        return GetGridRange(bb, bounds, minRange, maxRange);
    }

    bool NavigationGridSystem::GridToWorldSpace(GridSquare gridPos, Vector3& out) const
    {
        GridSquare maxRange = {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())};
        if (!checkInside(gridPos, {0, 0}, maxRange))
        {
            return false;
        }
        out = gridSquares[gridPos.row][gridPos.col]->worldPosMin; // Not centre?
        return true;
    }

    bool NavigationGridSystem::WorldToGridSpace(Vector3 worldPos, GridSquare& out) const
    {
        return WorldToGridSpace(
            worldPos,
            out,
            {0, 0},
            {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())});
    }

    bool NavigationGridSystem::WorldToGridSpace(
        Vector3 worldPos, GridSquare& out, const GridSquare& minRange, const GridSquare& maxRange) const
    {
        int x = std::floor(worldPos.x / spacing) + (slices / 2);
        int y = std::floor(worldPos.z / spacing) + (slices / 2);
        out = {y, x};

        return out.row < maxRange.row && out.col < maxRange.col && out.col >= minRange.col &&
               out.row >= minRange.row;
    }

    void NavigationGridSystem::DrawDebug() const
    {
        return;
        for (const auto& gridSquareRow : gridSquares)
        {
            for (const auto& gridSquare : gridSquareRow)
            {
                if (!gridSquare->drawDebug) continue;
                DrawCubeWires(
                    gridSquare->worldPosCentre,
                    gridSquare->debugBox.x,
                    gridSquare->debugBox.y,
                    gridSquare->debugBox.z,
                    gridSquare->debugColor);
            }
        }
    }

    std::vector<Vector3> NavigationGridSystem::tracebackPath(
        const std::vector<std::vector<GridSquare>>& came_from,
        const GridSquare& start,
        const GridSquare& finish) const
    {
        auto combineWorldPosTerrainHeight = [this](auto gridPos) {
            Vector3 worldPos = gridSquares[gridPos.row][gridPos.col]->worldPosMin;
            worldPos.y = gridSquares[gridPos.row][gridPos.col]->terrainHeight;
            return worldPos;
        };
        std::vector<Vector3> path;
        GridSquare current = {finish.row, finish.col};
        GridSquare previous;
        std::pair<int, int> currentDir = {0, 0};

        path.push_back(combineWorldPosTerrainHeight(current));
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
                        path.push_back(combineWorldPosTerrainHeight(previous));
                        path.push_back(combineWorldPosTerrainHeight(current));
                    }
                    break;
                }
            }
        }
        path.push_back(combineWorldPosTerrainHeight(current));
        std::ranges::reverse(path);
        return path;
    }

    bool NavigationGridSystem::checkInside(GridSquare square, GridSquare minRange, GridSquare maxRange)
    {
        return minRange.row <= square.row && square.row < maxRange.row && minRange.col <= square.col &&
               square.col < maxRange.col;
    }

    bool NavigationGridSystem::checkExtents(GridSquare square, GridSquare extents) const
    {
        auto min = square - extents;
        auto max = square + extents;

        for (int row = min.row; row < max.row; ++row)
        {
            for (int col = min.col; col < max.col; ++col)
            {
                if (gridSquares[row][col]->occupied)
                {
                    return false;
                }
            }
        }

        GridSquare minRange = {0, 0};
        GridSquare maxRange = {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())};
        return checkInside(min, minRange, maxRange) && checkInside(max, minRange, maxRange);
    }

    NavigationGridSquare* NavigationGridSystem::CastRay(
        int currentRow,
        int currentCol,
        Vector2 direction,
        float distance,
        std::vector<GridSquare>& debugLines) const
    {
        int dist = std::round(distance);
        direction = Vector2Normalize(direction);
        int dirRow = std::round(direction.y);
        int dirCol = std::round(direction.x);

        for (int i = 0; i < dist; ++i)
        {
            GridSquare square = {currentRow + (dirRow * i), currentCol + (dirCol * i)};
            debugLines.push_back(square);

            if (!checkInside(
                    square,
                    {0, 0},
                    {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())}))
            {
                continue;
            }

            const auto cell = gridSquares[square.row][square.col];
            cell->drawDebug = true;
            cell->debugColor = PURPLE;

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
        GridSquare currentPos;
        auto& trans = registry->get<sgTransform>(entity);
        if (!WorldToGridSpace(trans.GetWorldPos(), currentPos))
        {
            return {};
        }

        return FindNextBestLocation(currentPos, target, minRange, maxRange, extents);
    }

    GridSquare NavigationGridSystem::FindNextBestLocation(
        GridSquare currentPos,
        GridSquare target,
        GridSquare minRange,
        GridSquare maxRange,
        GridSquare extents) const
    {
        struct Compare
        {
            bool operator()(const std::pair<int, GridSquare>& a, const std::pair<int, GridSquare>& b)
            {
                return a.first > b.first; // Min-heap based on distance
            }
        };
        std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
        std::priority_queue<std::pair<int, GridSquare>, std::vector<std::pair<int, GridSquare>>, Compare> frontier;
        frontier.emplace(0, target);

        GridSquare out{};
        bool foundValidSquare = false;

        while (!frontier.empty())
        {
            auto currentPair = frontier.top();
            frontier.pop();
            auto current = currentPair.second;

            for (const auto& dir : directions)
            {
                GridSquare next = {current.row + dir.second, current.col + dir.first};

                if (!checkInside(next, minRange, maxRange)) continue;

                if (!checkExtents(next, extents))
                {
                    if (!visited[next.row][next.col])
                    {
                        frontier.emplace(heuristic(currentPos, next), next);
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
            if (foundValidSquare) break;
        }

        return out;
    }

    std::vector<Vector3> NavigationGridSystem::AStarPathfind(
        const entt::entity& entity,
        const Vector3& startPos,
        const Vector3& finishPos,
        AStarHeuristic heuristicType)
    {
        return AStarPathfind(
            entity,
            startPos,
            finishPos,
            {0, 0},
            {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())},
            heuristicType);
    }

    std::vector<Vector3> NavigationGridSystem::AStarPathfind(
        const entt::entity& entity,
        const Vector3& startPos,
        const Vector3& finishPos,
        const GridSquare& minRange,
        const GridSquare& maxRange,
        AStarHeuristic heuristicType)
    {
        GridSquare startGridSquare{};
        GridSquare finishGridSquare{};
        GridSquare extents{};

        if (!WorldToGridSpace(startPos, startGridSquare) || !WorldToGridSpace(finishPos, finishGridSquare) ||
            !getExtents(entity, extents))
            return {};

        if (!checkExtents(finishGridSquare, extents))
        {
            // TODO: Should try to find next best location to "original" destination
            finishGridSquare =
                FindNextBestLocation(startGridSquare, finishGridSquare, minRange, maxRange, extents);
        }

        std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
        std::vector<std::vector<GridSquare>> came_from(
            maxRange.row, std::vector<GridSquare>(maxRange.col, {-1, -1}));
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
                GridSquare next = {current.row + dir.first, current.col + dir.second};

                auto current_cost = gridSquares[current.row][current.col]->pathfindingCost;
                auto next_cost = gridSquares[next.row][next.col]->pathfindingCost;
                double new_cost = current_cost + next_cost;

                if (checkInside(next, minRange, maxRange) && checkExtents(next, extents) &&
                    (!visited[next.row][next.col] ||
                     (visited[next.row][next.col] && new_cost < cost_so_far[next.row][next.col])) &&
                    !gridSquares.at(next.row).at(next.col)->occupied)
                {
                    cost_so_far[next.row][next.col] = new_cost;
                    double heuristic_cost = heuristic(next, finishGridSquare);
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

    /**
     * Generates a sequence of nodes that should be the "optimal" route from point A to
     * point B. Checks entire grid.
     * @return A vector of "nodes" to travel to in sequential order. Empty if path is
     * invalid (OOB or no path available).
     */
    std::vector<Vector3> NavigationGridSystem::BFSPathfind(
        const entt::entity& entity, const Vector3& startPos, const Vector3& finishPos)
    {
        return BFSPathfind(
            entity,
            startPos,
            finishPos,
            {0, 0},
            {static_cast<int>(gridSquares.at(0).size()), static_cast<int>(gridSquares.size())});
    }

    /**
     * Generates a sequence of nodes that should be the "optimal" route from point A to
     * point B. Checks path within a range. Use "GetPathfindRange" to calculate
     * minRange/maxRange if needed.
     * @minRange The minimum grid index in the pathfinding range.
     * @maxRange The maximum grid index in the pathfinding range.
     * @return A vector of "nodes" to travel to in sequential order. Empty if path is
     * invalid (OOB or no path available).
     */
    std::vector<Vector3> NavigationGridSystem::BFSPathfind(
        const entt::entity& entity,
        const Vector3& startPos,
        const Vector3& finishPos,
        const GridSquare& minRange,
        const GridSquare& maxRange)
    {
        GridSquare start{};
        GridSquare finish{};
        GridSquare extents{};
        if (!WorldToGridSpace(startPos, start) || !WorldToGridSpace(finishPos, finish) ||
            !getExtents(entity, extents))
            return {};

        if (!checkExtents(finish, extents))
        {
            // TODO: Should actually try to find next best location to original
            // destination
            finish = FindNextBestLocation(start, finish, minRange, maxRange, extents);
        }

        std::vector<std::vector<bool>> visited(maxRange.row, std::vector<bool>(maxRange.col, false));
        std::vector<std::vector<GridSquare>> came_from(
            maxRange.row, std::vector<GridSquare>(maxRange.col, {-1, -1}));

        std::queue<GridSquare> frontier;

        frontier.emplace(start);
        visited[start.row][start.col] = true;

        bool pathFound = false;

        while (!frontier.empty())
        {
            auto current = frontier.front();
            frontier.pop();

            if (current.row == finish.row && current.col == finish.col)
            {
                pathFound = true;
                break;
            }

            for (const auto& dir : directions)
            {
                GridSquare next = {current.row + dir.first, current.col + dir.second};

                if (checkInside(next, minRange, maxRange) && !visited[next.row][next.col] &&
                    checkExtents(next, extents) && !gridSquares[next.row][next.col]->occupied)
                {
                    frontier.emplace(next);
                    visited[next.row][next.col] = true;
                    came_from[next.row][next.col] = current;
                }
            }
        }

        if (!pathFound)
        {
            return {};
        }

        return tracebackPath(came_from, start, finish);
    }

    void NavigationGridSystem::InitGridHeightNormals()
    {
        std::cout << "Initialising grid height and normals \n";
        const auto& view = registry->view<Collideable, Renderable>();
        for (const auto& entity : view)
        {
            const auto& bb = view.get<Collideable>(entity);

            if (bb.collisionLayer == CollisionLayer::FLOOR)
            {
                calculateTerrainHeightAndNormals(entity);
            }
        }
    }

    /*
     * This function finds the collisions between buildings in the world and the grid
     * squares and marks them as occupied.
     */
    void NavigationGridSystem::PopulateGrid(ImageSafe& heightMap, ImageSafe& normalMap)
    {
        // TODO: The below crashes when trying to autogenerate the amount of slices
        for (auto& row : gridSquares)
        {
            for (auto& gridSquare : row)
            {
                gridSquare->occupied = false;
            }
        }

        const auto& view = registry->view<Collideable, Renderable>();
        // Load from image data
        loadTerrainNormalMap(normalMap);
        loadTerrainHeightMap(heightMap, getMaxHeight(slices));

        std::cout << "Populating grid started. \n";
        for (const auto& entity : view)
        {
            const auto& bb = view.get<Collideable>(entity);

            if (bb.collisionLayer == CollisionLayer::BUILDING)
            {
                MarkSquareAreaOccupied(bb.worldBoundingBox, true, entity);
            }
            else if (bb.collisionLayer == CollisionLayer::FLOOR)
            {
                MarkSquareAreaOccupiedIfSteep(bb.worldBoundingBox, true);
            }
        }
        std::cout << "Populating grid finished. \n";
    }

    const std::vector<std::vector<NavigationGridSquare*>>& NavigationGridSystem::GetGridSquares()
    {
        return gridSquares;
    }

    const NavigationGridSquare* NavigationGridSystem::GetGridSquare(int row, int col) const
    {
        return gridSquares[row][col];
    }

    NavigationGridSystem::NavigationGridSystem(entt::registry* _registry, CollisionSystem* _collisionSystem)
        : BaseSystem(_registry), collisionSystem(_collisionSystem)
    {
    }
} // namespace sage
