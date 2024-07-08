//
// Created by Steve Wheeler on 03/05/2024.
//

#include "Renderable.hpp"

#include <utility>
#include <cstring>

namespace sage
{
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
