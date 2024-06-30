//
// Created by Steve Wheeler on 03/05/2024.
//

#include "Renderable.hpp"

#include <utility>
#include <cstring>

namespace sage
{
	BoundingBox Renderable::CalculateModelBoundingBox() const
	{
		Mesh mesh = model.meshes[0];
		std::vector<float> vertices(mesh.vertexCount * 3);
		memcpy(&vertices[0], mesh.vertices, sizeof(float) * mesh.vertexCount * 3);

		BoundingBox bb;
		bb.min = {0, 0, 0};
		bb.max = {0, 0, 0};

		{
			float x = vertices[0];
			float y = vertices[1];
			float z = vertices[2];

			Vector3 v = {x, y, z};
			// Assuming rl.Vector3Transform is a function that transforms a Vector3
			// using the given transform.
			v = Vector3Transform(v, model.transform);

			bb.min = bb.max = v;
		}

		for (size_t i = 0; i < vertices.size(); i += 3)
		{
			float x = vertices[i];
			float y = vertices[i + 1];
			float z = vertices[i + 2];

			Vector3 v = {x, y, z};
			v = Vector3Transform(v, model.transform);

			bb.min.x = std::min(bb.min.x, v.x);
			bb.min.y = std::min(bb.min.y, v.y);
			bb.min.z = std::min(bb.min.z, v.z);

			bb.max.x = std::max(bb.max.x, v.x);
			bb.max.y = std::max(bb.max.y, v.y);
			bb.max.z = std::max(bb.max.z, v.z);
		}

		return bb;
	}

	Renderable::~Renderable()
	{
		if (shader.has_value())
		{
			UnloadShader(shader.value());
		}
		UnloadModel(model);
		UnloadTexture(material.diffuse);
	}

	Renderable::Renderable(
		Model _model,
		Material _material,
		std::string _modelPath,
		Matrix _localTransform)
		:
		initialTransform(_localTransform),
		material(std::move(_material)),
		modelPath(std::move(_modelPath)),
		model(_model)
	{
		model.transform = initialTransform;
	}

	Renderable::Renderable(Model _model, std::string _modelPath, Matrix _localTransform) :
		initialTransform(_localTransform), modelPath(std::move(_modelPath)), model(_model)
	{
		model.transform = initialTransform;
	}
}
