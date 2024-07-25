//
// Created by Steve Wheeler on 25/07/2024.
//

#include "TextureTerrainOverlay.hpp"

#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"

namespace sage
{

	Model TextureTerrainOverlay::generateTerrainPolygon()
	{
		return LoadModelFromMesh(GenMeshPoly(5, 2.0f));
		Mesh mesh;
		auto gridSquares = navigationGridSystem->GetGridSquares();
		mesh.vertices = (float*)RL_MALLOC(sizeof(float) * 3 * gridSquares.size());
		mesh.normals = (float*)RL_MALLOC(sizeof(float) * 3 * gridSquares.size());
		int maxRow = maxRange.row - minRange.row;
		int maxCol = maxRange.col - minRange.col;
		int i = 0;
		for (int row = minRange.row; row < maxRow; ++row)
		{
			for (int col = minRange.col; col < maxCol; ++i, ++col)
			{
				mesh.vertices[i * 3] = gridSquares[row][col]->worldPosMin.x;
				mesh.vertices[i * 3 + 1] = gridSquares[row][col]->terrainHeight;
				mesh.vertices[i * 3 + 2] = gridSquares[row][col]->worldPosMin.z;
				mesh.normals[i * 3] = gridSquares[row][col]->terrainNormal.x;
				mesh.normals[i * 3 + 1] = gridSquares[row][col]->terrainNormal.y;
				mesh.normals[i * 3 + 2] = gridSquares[row][col]->terrainNormal.z;
			}
		}
		mesh.triangleCount = i;
		mesh.vertexCount = i*3;
		Model _model = LoadModelFromMesh(mesh);
		_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
		return _model;
	}

	TextureTerrainOverlay::~TextureTerrainOverlay()
	{
		UnloadTexture(texture);
	}

	TextureTerrainOverlay::TextureTerrainOverlay(
			entt::registry* _registry,
			NavigationGridSystem* _navigationGridSystem,
			const char* texturePath)
			:
			registry(_registry),
			navigationGridSystem(_navigationGridSystem)
	{
		texture = LoadTexture(texturePath);
		entity = registry->create();

		registry->emplace<Renderable>(entity);
		registry->emplace<sgTransform>(entity);
	}

	void TextureTerrainOverlay::Init(Vector3 mouseRayHit)
	{
		navigationGridSystem->WorldToGridSpace(mouseRayHit, lastHit);
		navigationGridSystem->GetGridRange(mouseRayHit, 10, minRange, maxRange);
		auto& renderable = registry->get<Renderable>(entity);
		renderable.model = generateTerrainPolygon();
		auto topLeft = navigationGridSystem->GetGridSquare(minRange.row, minRange.col);
		auto& trans = registry->get<sgTransform>(entity);
		trans.SetPosition({ topLeft->worldPosMin.x, topLeft->terrainHeight, topLeft->worldPosMin.z }, entity);
//		Matrix matrix = MatrixTranslate(topLeft->worldPosMin.x, topLeft->terrainHeight, topLeft->worldPosMin.z);
//		renderable.model.transform = matrix;
	}

	void TextureTerrainOverlay::Update(Vector3 mouseRayHit)
	{
		GridSquare mousePosGrid{};
		navigationGridSystem->WorldToGridSpace(mouseRayHit, mousePosGrid);
		if (lastHit == mousePosGrid) return;
		Init(mouseRayHit);
	}
} // sage