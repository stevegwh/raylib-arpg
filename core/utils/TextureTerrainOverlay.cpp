#include "TextureTerrainOverlay.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"

namespace sage
{

    void TextureTerrainOverlay::updateMeshData(Mesh& mesh)
    {
        int maxRow = maxRange.row - minRange.row;
        int maxCol = maxRange.col - minRange.col;
        Vector3 centerPos = calculateCenterPosition();

        for (int row = 0; row < maxRow; ++row)
        {
            for (int col = 0; col < maxCol; ++col)
            {
                int vertexIndex = row * maxCol + col;
                int gridRow = row + minRange.row;
                int gridCol = col + minRange.col;

                updateVertexData(mesh, vertexIndex, gridRow, gridCol, centerPos);
                updateNormalData(mesh, vertexIndex, gridRow, gridCol);
                updateTexCoordData(mesh, vertexIndex, row, col, maxRow, maxCol);
            }
        }
    }

    Mesh TextureTerrainOverlay::createInitialMesh()
    {
        int maxRow = maxRange.row - minRange.row;
        int maxCol = maxRange.col - minRange.col;
        int vertexCount = maxRow * maxCol;

        Mesh mesh = {0};
        mesh.vertexCount = vertexCount;
        mesh.triangleCount = (maxRow - 1) * (maxCol - 1) * 2;
        mesh.vertices = (float*)RL_MALLOC(vertexCount * 3 * sizeof(float));
        mesh.normals = (float*)RL_MALLOC(vertexCount * 3 * sizeof(float));
        mesh.texcoords = (float*)RL_MALLOC(vertexCount * 2 * sizeof(float));
        mesh.indices = (unsigned short*)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

        updateMeshData(mesh);
        generateIndices(mesh, maxRow, maxCol);

        return mesh;
    }

    void TextureTerrainOverlay::updateVertexData(
        Mesh& mesh, int vertexIndex, int gridRow, int gridCol, const Vector3& centerPos)
    {
        const auto& gridSquares = navigationGridSystem->GetGridSquares();
        mesh.vertices[vertexIndex * 3] = gridSquares[gridRow][gridCol]->worldPosMin.x - centerPos.x;
        mesh.vertices[vertexIndex * 3 + 1] = gridSquares[gridRow][gridCol]->terrainHeight;
        mesh.vertices[vertexIndex * 3 + 2] = gridSquares[gridRow][gridCol]->worldPosMin.z - centerPos.z;
    }

    void TextureTerrainOverlay::updateNormalData(Mesh& mesh, int vertexIndex, int gridRow, int gridCol)
    {
        // TODO: I think the mesh created is actually really big, that's why this offset is so weird
        // the grid "range" could be vastly reduced.
        const auto& gridSquares = navigationGridSystem->GetGridSquares();
        mesh.normals[vertexIndex * 3] = gridSquares[gridRow][gridCol]->terrainNormal.x;
        mesh.normals[vertexIndex * 3 + 1] = gridSquares[gridRow][gridCol]->terrainNormal.y;
        mesh.normals[vertexIndex * 3 + 2] = gridSquares[gridRow][gridCol]->terrainNormal.z;
    }

    void TextureTerrainOverlay::updateTexCoordData(
        Mesh& mesh, int vertexIndex, int row, int col, int maxRow, int maxCol)
    {
        mesh.texcoords[vertexIndex * 2] = (float)col / (maxCol - 1);
        mesh.texcoords[vertexIndex * 2 + 1] = (float)row / (maxRow - 1);
    }

    void TextureTerrainOverlay::generateIndices(Mesh& mesh, int maxRow, int maxCol)
    {
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
    }

    Vector3 TextureTerrainOverlay::calculateCenterPosition()
    {
        // TODO: I think the height value in this function is causing all the issues
        // Also, what happens if the shape is concave?
        const auto& gridSquares = navigationGridSystem->GetGridSquares();
        return {
            (gridSquares[minRange.row][minRange.col]->worldPosMin.x +
             gridSquares[maxRange.row - 1][maxRange.col - 1]->worldPosMin.x) *
                0.5f,
            (gridSquares[minRange.row][minRange.col]->terrainHeight +
             gridSquares[maxRange.row - 1][maxRange.col - 1]->terrainHeight) *
                0.5f,
            (gridSquares[minRange.row][minRange.col]->worldPosMin.z +
             gridSquares[maxRange.row - 1][maxRange.col - 1]->worldPosMin.z) *
                0.5f};
    }

    void TextureTerrainOverlay::updateTerrainPolygon()
    {
        auto& mesh = *registry->get<Renderable>(entity).model.meshes;
        updateMeshData(mesh);

        int vertexCount = mesh.vertexCount;
        UpdateMeshBuffer(mesh, 0, mesh.vertices, vertexCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(mesh, 1, mesh.normals, vertexCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(mesh, 2, mesh.texcoords, vertexCount * 2 * sizeof(float), 0);
    }

    Model TextureTerrainOverlay::generateTerrainPolygon()
    {
        Mesh mesh = createInitialMesh();

        UploadMesh(&mesh, false);
        Model model = LoadModelFromMesh(mesh);

        model.materials = (Material*)RL_MALLOC(sizeof(Material));
        model.materials[0] = LoadMaterialDefault();
        model.materialCount = 1;
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

        return model;
    }

    void TextureTerrainOverlay::Enable(bool enable)
    {
        assert(initialised);
        m_active = enable;
        auto& renderable = registry->get<Renderable>(entity);
        renderable.active = enable;
    }

    void TextureTerrainOverlay::Init(Vector3 mouseRayHit) // TODO: Should take radius as a parameter
    {
        if (initialised) return;
        initialised = true;

        navigationGridSystem->WorldToGridSpace(mouseRayHit, lastHit);
        navigationGridSystem->GetGridRange(mouseRayHit, 10, minRange, maxRange);
        auto& renderable = registry->get<Renderable>(entity);
        renderable.model = generateTerrainPolygon();
        renderable.model.materials[0].shader = renderable.shader.value();

        auto& trans = registry->get<sgTransform>(entity);
        Vector3 centerPos = calculateCenterPosition();
        centerPos.y = mouseRayHit.y;
        trans.SetPosition(centerPos, entity);
        renderable.model.transform = MatrixIdentity();
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
        Vector3 centerPos = calculateCenterPosition();
        centerPos.y = mouseRayHit.y;
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