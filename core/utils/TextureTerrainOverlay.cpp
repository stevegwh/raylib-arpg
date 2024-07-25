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
		Mesh mesh = { 0 };

		auto gridSquares = navigationGridSystem->GetGridSquares();
		int maxRow = maxRange.row - minRange.row;
		int maxCol = maxRange.col - minRange.col;
		int vertexCount = maxRow * maxCol;
		int triangleCount = (maxRow - 1) * (maxCol - 1) * 2;

		mesh.vertices = (float*)RL_MALLOC(vertexCount * 3 * sizeof(float));
		mesh.normals = (float*)RL_MALLOC(vertexCount * 3 * sizeof(float));
		mesh.texcoords = (float*)RL_MALLOC(vertexCount * 2 * sizeof(float));
		mesh.colors = (unsigned char*)RL_MALLOC(vertexCount * 4 * sizeof(unsigned char)); // Add colors
		mesh.indices = (unsigned short*)RL_MALLOC(triangleCount * 3 * sizeof(unsigned short));
		int vertexIndex = 0;
		for (int row = minRange.row; row < maxRange.row; ++row)
		{
			for (int col = minRange.col; col < maxRange.col; ++col, ++vertexIndex)
			{
				mesh.vertices[vertexIndex * 3] = gridSquares[row][col]->worldPosMin.x;
				mesh.vertices[vertexIndex * 3 + 1] = gridSquares[row][col]->terrainHeight;
				mesh.vertices[vertexIndex * 3 + 2] = gridSquares[row][col]->worldPosMin.z;

				mesh.normals[vertexIndex * 3] = gridSquares[row][col]->terrainNormal.x;
				mesh.normals[vertexIndex * 3 + 1] = gridSquares[row][col]->terrainNormal.y;
				mesh.normals[vertexIndex * 3 + 2] = gridSquares[row][col]->terrainNormal.z;

				mesh.texcoords[vertexIndex * 2] = (float)(col - minRange.col) / (maxCol - 1);
				mesh.texcoords[vertexIndex * 2 + 1] = (float)(row - minRange.row) / (maxRow - 1);
			}
		}

		mesh.vertexCount = vertexCount;
		mesh.triangleCount = triangleCount;
		UploadMesh(&mesh, false);
		Model _model = LoadModelFromMesh(mesh);
		if (_model.materials == NULL)
		{
			_model.materials = (Material*)RL_MALLOC(sizeof(Material));
			_model.materials[0] = LoadMaterialDefault();
			_model.materialCount = 1;
		}
		//_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

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
		auto& trans = registry->get<sgTransform>(entity);
		auto centre = navigationGridSystem->GetGridSquare(lastHit.row, lastHit.col);
		trans.SetPosition({ centre->worldPosMin.x, centre->terrainHeight, centre->worldPosMin.z }, entity);
		renderable.model.transform = MatrixIdentity();
	}

	void TextureTerrainOverlay::Update(Vector3 mouseRayHit)
	{
		GridSquare mousePosGrid{};
		navigationGridSystem->WorldToGridSpace(mouseRayHit, mousePosGrid);
		if (lastHit == mousePosGrid) return;
		Init(mouseRayHit);
	}
} // sage