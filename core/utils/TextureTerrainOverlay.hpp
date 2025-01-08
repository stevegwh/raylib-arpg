#pragma once

#include "components/NavigationGridSquare.hpp"
#include "raylib.h"
#include "systems/NavigationGridSystem.hpp"
#include "entt/entt.hpp"

namespace sage
{
    class TextureTerrainOverlay
    {
        entt::registry* registry;
        NavigationGridSystem* navigationGridSystem;
        Texture2D texture;
        const entt::entity entity;
        GridSquare lastHit{};
        bool initialised = false;
        bool m_active = false;
        Vector3 meshOffset{};
        float radius{};

        void updateTerrainPolygon(const GridSquare& minRange, const GridSquare& maxRange) const;
        Model generateTerrainPolygon(const GridSquare& minRange, const GridSquare& maxRange);
        void updateMeshData(Mesh& mesh, const GridSquare& minRange, const GridSquare& maxRange) const;
        Mesh createInitialMesh(const GridSquare& minRange, const GridSquare& maxRange);
        void updateVertexData(Mesh& mesh, int vertexIndex, int gridRow, int gridCol) const;
        void updateNormalData(Mesh& mesh, int vertexIndex, int gridRow, int gridCol) const;
        static void updateTexCoordData(Mesh& mesh, int vertexIndex, int row, int col, int maxRow, int maxCol);
        static void generateIndices(Mesh& mesh, int maxRow, int maxCol);

      public:
        void SetHint(Color col);
        void SetShader(Shader shader);
        void Enable(bool enable);
        void Init(Vector3 startPos, float _radius = 10);
        [[nodiscard]] bool IsActive() const;
        void Update(Vector3 pos);
        ~TextureTerrainOverlay();
        TextureTerrainOverlay(
            entt::registry* _registry,
            NavigationGridSystem* _navigationGridSystem,
            Texture tex,
            Color _hint,
            const char* shaderPath);
        TextureTerrainOverlay(
            entt::registry* _registry,
            NavigationGridSystem* _navigationGridSystem,
            AssetID assetId,
            Color _hint,
            const char* shaderPath);
        TextureTerrainOverlay(
            entt::registry* _registry,
            NavigationGridSystem* _navigationGridSystem,
            AssetID assetId,
            Color _hint,
            Shader _shader);
    };

} // namespace sage