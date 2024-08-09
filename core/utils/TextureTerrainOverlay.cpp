//
// Created by Steve Wheeler on 25/07/2024.
//

#include "TextureTerrainOverlay.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"

namespace sage
{

    void TextureTerrainOverlay::updateTerrainPolygon()
    {
        int maxRow = maxRange.row - minRange.row;
        int maxCol = maxRange.col - minRange.col;
        int vertexCount = maxRow * maxCol;
        auto gridSquares = navigationGridSystem->GetGridSquares();
        auto& mesh = *registry->get<Renderable>(entity).model.meshes;

        // Calculate the center of the mesh
        Vector3 centerPos = {
            (gridSquares[minRange.row][minRange.col]->worldPosMin.x +
             gridSquares[maxRange.row - 1][maxRange.col - 1]->worldPosMin.x) *
                0.5f,
            (gridSquares[minRange.row][minRange.col]->terrainHeight +
             gridSquares[maxRange.row - 1][maxRange.col - 1]->terrainHeight) *
                0.5f,
            (gridSquares[minRange.row][minRange.col]->worldPosMin.z +
             gridSquares[maxRange.row - 1][maxRange.col - 1]->worldPosMin.z) *
                0.5f};

        // Update vertices, normals, and texcoords
        for (int row = 0; row < maxRow; ++row)
        {
            for (int col = 0; col < maxCol; ++col)
            {
                int vertexIndex = row * maxCol + col;
                int gridRow = row + minRange.row;
                int gridCol = col + minRange.col;

                mesh.vertices[vertexIndex * 3] =
                    gridSquares[gridRow][gridCol]->worldPosMin.x - centerPos.x;
                mesh.vertices[vertexIndex * 3 + 1] =
                    gridSquares[gridRow][gridCol]->terrainHeight - centerPos.y;
                mesh.vertices[vertexIndex * 3 + 2] =
                    gridSquares[gridRow][gridCol]->worldPosMin.z - centerPos.z;

                mesh.normals[vertexIndex * 3] =
                    gridSquares[gridRow][gridCol]->terrainNormal.x;
                mesh.normals[vertexIndex * 3 + 1] =
                    gridSquares[gridRow][gridCol]->terrainNormal.y;
                mesh.normals[vertexIndex * 3 + 2] =
                    gridSquares[gridRow][gridCol]->terrainNormal.z;

                mesh.texcoords[vertexIndex * 2] = (float)col / (maxCol - 1);
                mesh.texcoords[vertexIndex * 2 + 1] = (float)row / (maxRow - 1);
            }
        }

        // Update the mesh on the GPU
        UpdateMeshBuffer(mesh, 0, mesh.vertices, vertexCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(mesh, 1, mesh.normals, vertexCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(mesh, 2, mesh.texcoords, vertexCount * 2 * sizeof(float), 0);
    }

    Model TextureTerrainOverlay::generateTerrainPolygon()
    {
        Mesh mesh = {0};

        auto gridSquares = navigationGridSystem->GetGridSquares();
        int maxRow = maxRange.row - minRange.row;
        int maxCol = maxRange.col - minRange.col;
        int vertexCount = maxRow * maxCol;
        int triangleCount = (maxRow - 1) * (maxCol - 1) * 2;

        mesh.vertexCount = vertexCount;
        mesh.triangleCount = triangleCount;

        mesh.vertices = (float*)RL_MALLOC(vertexCount * 3 * sizeof(float));
        mesh.normals = (float*)RL_MALLOC(vertexCount * 3 * sizeof(float));
        mesh.texcoords = (float*)RL_MALLOC(vertexCount * 2 * sizeof(float));
        mesh.indices =
            (unsigned short*)RL_MALLOC(triangleCount * 3 * sizeof(unsigned short));

        // Fill vertices, normals, and texcoords
        int vertexIndex = 0;
        for (int row = minRange.row; row < maxRange.row; ++row)
        {
            for (int col = minRange.col; col < maxRange.col; ++col, ++vertexIndex)
            {
                mesh.vertices[vertexIndex * 3] = gridSquares[row][col]->worldPosMin.x;
                mesh.vertices[vertexIndex * 3 + 1] = gridSquares[row][col]->terrainHeight;
                mesh.vertices[vertexIndex * 3 + 2] = gridSquares[row][col]->worldPosMin.z;

                mesh.normals[vertexIndex * 3] = gridSquares[row][col]->terrainNormal.x;
                mesh.normals[vertexIndex * 3 + 1] =
                    gridSquares[row][col]->terrainNormal.y;
                mesh.normals[vertexIndex * 3 + 2] =
                    gridSquares[row][col]->terrainNormal.z;

                mesh.texcoords[vertexIndex * 2] =
                    (float)(col - minRange.col) / (maxCol - 1);
                mesh.texcoords[vertexIndex * 2 + 1] =
                    (float)(row - minRange.row) / (maxRow - 1);
            }
        }

        // Generate indices for triangles
        int indexCount = 0;
        for (int row = 0; row < maxRow - 1; ++row)
        {
            for (int col = 0; col < maxCol - 1; ++col)
            {
                int topLeft = row * maxCol + col;
                int topRight = topLeft + 1;
                int bottomLeft = (row + 1) * maxCol + col;
                int bottomRight = bottomLeft + 1;

                mesh.indices[indexCount++] = topLeft;
                mesh.indices[indexCount++] = bottomLeft;
                mesh.indices[indexCount++] = topRight;

                mesh.indices[indexCount++] = topRight;
                mesh.indices[indexCount++] = bottomLeft;
                mesh.indices[indexCount++] = bottomRight;
            }
        }

        UploadMesh(&mesh, false);
        Model model = LoadModelFromMesh(mesh);

        // Set up material
        model.materials = (Material*)RL_MALLOC(sizeof(Material));
        model.materials[0] = LoadMaterialDefault();
        model.materialCount = 1;
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

        return model;
    }

    void TextureTerrainOverlay::Enable(bool enable)
    {
        m_active = enable;
        auto& renderable = registry->get<Renderable>(entity);
        renderable.active = enable;
    }

    void TextureTerrainOverlay::Init(Vector3 mouseRayHit)
    {
        if (initialised)
        {
            return;
        }
        else
        {
            initialised = true;
        }
        navigationGridSystem->WorldToGridSpace(mouseRayHit, lastHit);
        navigationGridSystem->GetGridRange(mouseRayHit, 10, minRange, maxRange);
        auto& renderable = registry->get<Renderable>(entity);
        renderable.model = generateTerrainPolygon();
        renderable.model.materials[0].shader = renderable.shader.value();
        renderable.model.transform = MatrixIdentity();
        auto& trans = registry->get<sgTransform>(entity);
        auto centre = navigationGridSystem->GetGridSquare(lastHit.row, lastHit.col);
        trans.SetPosition(
            {centre->worldPosMin.x, centre->terrainHeight, centre->worldPosMin.z},
            entity);
    }

    bool TextureTerrainOverlay::active() const
    {
        return m_active;
    }

    void TextureTerrainOverlay::Update(Vector3 mouseRayHit)
    {
        if (!m_active) return;
        GridSquare mousePosGrid{};
        navigationGridSystem->WorldToGridSpace(mouseRayHit, mousePosGrid);
        if (lastHit == mousePosGrid) return;

        lastHit = mousePosGrid;
        navigationGridSystem->GetGridRange(mouseRayHit, 10, minRange, maxRange);

        auto& renderable = registry->get<Renderable>(entity);
        updateTerrainPolygon();

        auto& trans = registry->get<sgTransform>(entity);
        // Calculate the center of the mesh
        Vector3 centerPos = {
            (navigationGridSystem->GetGridSquare(minRange.row, minRange.col)
                 ->worldPosMin.x +
             navigationGridSystem->GetGridSquare(maxRange.row - 1, maxRange.col - 1)
                 ->worldPosMin.x) *
                0.5f,
            mouseRayHit.y, // Use the mouseRayHit.y for height
            (navigationGridSystem->GetGridSquare(minRange.row, minRange.col)
                 ->worldPosMin.z +
             navigationGridSystem->GetGridSquare(maxRange.row - 1, maxRange.col - 1)
                 ->worldPosMin.z) *
                0.5f};

        trans.SetPosition(centerPos, entity);
        renderable.model.transform = MatrixIdentity();
    }

    TextureTerrainOverlay::~TextureTerrainOverlay()
    {
        UnloadTexture(texture);
    }

    TextureTerrainOverlay::TextureTerrainOverlay(
        entt::registry* _registry,
        NavigationGridSystem* _navigationGridSystem,
        const char* texturePath,
        Color _hint,
        const char* shaderPath)
        : registry(_registry),
          navigationGridSystem(_navigationGridSystem),
          entity(_registry->create()),
          texture(LoadTexture(texturePath))
    {
        auto& r = registry->emplace<Renderable>(entity);
        r.shader = std::make_optional(LoadShader(nullptr, shaderPath));
        r.hint = _hint;
        registry->emplace<sgTransform>(entity);
    }
} // namespace sage