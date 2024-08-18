#pragma once

#include "components/NavigationGridSquare.hpp"
#include "raylib.h"
#include "systems/NavigationGridSystem.hpp"
#include <entt/entity/registry.hpp>

namespace sage
{

    class TextureTerrainOverlay
    {
      private:
        entt::registry* registry;
        NavigationGridSystem* navigationGridSystem;
        Texture2D texture;
        GridSquare lastHit;
        GridSquare minRange, maxRange;
        bool initialised = false;
        bool m_active = false;

        void updateTerrainPolygon();
        Model generateTerrainPolygon();
        void updateMeshData(Mesh& mesh);
        Mesh createInitialMesh();
        void updateVertexData(Mesh& mesh, int vertexIndex, int gridRow, int gridCol, const Vector3& centerPos);
        void updateNormalData(Mesh& mesh, int vertexIndex, int gridRow, int gridCol);
        void updateTexCoordData(Mesh& mesh, int vertexIndex, int row, int col, int maxRow, int maxCol);
        void generateIndices(Mesh& mesh, int maxRow, int maxCol);
        Vector3 calculateCenterPosition();

      public:
        const entt::entity entity;

        void Enable(bool enable);
        void Init(Vector3 mouseRayHit);
        bool active() const;
        void Update(Vector3 mouseRayHit);
        ~TextureTerrainOverlay();
        TextureTerrainOverlay(
            entt::registry* _registry,
            NavigationGridSystem* _navigationGridSystem,
            const char* texturePath,
            Color _hint,
            const char* shaderPath);
    };

} // namespace sage