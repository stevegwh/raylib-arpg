#include "TextureTerrainOverlay.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "systems/NavigationGridSystem.hpp"

namespace sage
{

    void TextureTerrainOverlay::updateMeshData(Mesh& mesh)
    {
        int maxRow = maxRange.row - minRange.row;
        int maxCol = maxRange.col - minRange.col;

        for (int row = 0; row < maxRow; ++row)
        {
            for (int col = 0; col < maxCol; ++col)
            {
                int vertexIndex = row * maxCol + col;
                int gridRow = row + minRange.row;
                int gridCol = col + minRange.col;

                updateVertexData(mesh, vertexIndex, gridRow, gridCol);
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

    void TextureTerrainOverlay::updateVertexData(Mesh& mesh, int vertexIndex, int gridRow, int gridCol)
    {
        const auto& gridSquares = navigationGridSystem->GetGridSquares();
        mesh.vertices[vertexIndex * 3] = gridSquares[gridRow][gridCol]->worldPosMin.x;
        mesh.vertices[vertexIndex * 3 + 1] = gridSquares[gridRow][gridCol]->terrainHeight +
                                             0.3; // Little buffer so the overlay doesnt blend into terrain
        mesh.vertices[vertexIndex * 3 + 2] = gridSquares[gridRow][gridCol]->worldPosMin.z;
    }

    void TextureTerrainOverlay::updateNormalData(Mesh& mesh, int vertexIndex, int gridRow, int gridCol)
    {
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

    void TextureTerrainOverlay::updateTerrainPolygon()
    {
        auto& mesh = *registry->get<Renderable>(entity).GetModel()->rlmodel.meshes;
        updateMeshData(mesh);

        int vertexCount = mesh.vertexCount;
        UpdateMeshBuffer(mesh, 0, mesh.vertices, vertexCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(mesh, 1, mesh.normals, vertexCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(mesh, 2, mesh.texcoords, vertexCount * 2 * sizeof(float), 0);
    }

    ModelSafe TextureTerrainOverlay::generateTerrainPolygon()
    {
        Mesh mesh = createInitialMesh();

        UploadMesh(&mesh, false);
        Model model = LoadModelFromMesh(mesh);

        model.materials = (Material*)RL_MALLOC(sizeof(Material));
        model.materials[0] = LoadMaterialDefault();
        model.materialCount = 1;
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

        return ModelSafe{model};
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
        if (initialised)
        {
            Update(mouseRayHit);
            return;
        };
        initialised = true;

        navigationGridSystem->WorldToGridSpace(mouseRayHit, lastHit);
        navigationGridSystem->GetGridRange(mouseRayHit, 10, minRange, maxRange);

        // Calculate the center of the mesh in world space
        const auto& gridSquares = navigationGridSystem->GetGridSquares();
        Vector3 meshMin = {
            gridSquares[minRange.row][minRange.col]->worldPosMin.x,
            gridSquares[minRange.row][minRange.col]->terrainHeight,
            gridSquares[minRange.row][minRange.col]->worldPosMin.z};
        Vector3 meshMax = {
            gridSquares[maxRange.row - 1][maxRange.col - 1]->worldPosMax.x,
            gridSquares[maxRange.row - 1][maxRange.col - 1]->terrainHeight,
            gridSquares[maxRange.row - 1][maxRange.col - 1]->worldPosMax.z};
        Vector3 meshCenter = {(meshMin.x + meshMax.x) * 0.5f, 0, (meshMin.z + meshMax.z) * 0.5f};

        // Calculate the offset to center the mesh on the mouse position
        meshOffset = {mouseRayHit.x - meshCenter.x, 0, mouseRayHit.z - meshCenter.z};

        auto& trans = registry->get<sgTransform>(entity);
        trans.SetPosition(meshOffset);
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
        trans.SetPosition(meshOffset);
    }

    TextureTerrainOverlay::~TextureTerrainOverlay()
    {
        registry->remove<TextureTerrainOverlay>(entity);
    }

    TextureTerrainOverlay::TextureTerrainOverlay(
        entt::registry* _registry,
        NavigationGridSystem* _navigationGridSystem,
        const char* texturePath,
        Color _hint,
        Shader shader)
        : registry(_registry),
          navigationGridSystem(_navigationGridSystem),
          entity(_registry->create()),
          texture(ResourceManager::GetInstance().TextureLoad(texturePath))
    {
        auto& renderable =
            registry->emplace<Renderable>(entity, std::move(generateTerrainPolygon()), MatrixIdentity());

        renderable.GetModel()->SetShader(shader, 0);

        renderable.hint = _hint;
        registry->emplace<sgTransform>(entity, entity);
        registry->emplace<RenderableDeferred>(entity);
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
          texture(ResourceManager::GetInstance().TextureLoad(texturePath))
    {
        auto& renderable =
            registry->emplace<Renderable>(entity, std::move(generateTerrainPolygon()), MatrixIdentity());

        renderable.GetModel()->SetShader(ResourceManager::GetInstance().ShaderLoad(nullptr, shaderPath), 0);

        renderable.hint = _hint;
        registry->emplace<sgTransform>(entity, entity);
        registry->emplace<RenderableDeferred>(entity);
    }

} // namespace sage