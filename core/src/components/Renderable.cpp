#include "Renderable.hpp"
//
// Created by Steve Wheeler on 03/05/2024.
//

#include "Renderable.hpp"

#include <cstring>
#include <utility>

namespace sage
{
    Renderable::~Renderable()
    {
        if (shader.has_value())
        {
            UnloadShader(shader.value());
        }
        UnloadModel(model);
        for (auto& texture : textures)
        {
            UnloadTexture(texture);
        }
    }

    Renderable::Renderable(Model _model, MaterialPaths _materials, Matrix _localTransform)
        : initialTransform(_localTransform), materials(std::move(_materials)), model(_model)
    {
        model.transform = initialTransform;
    }

    Renderable::Renderable(Model _model, Matrix _localTransform) : initialTransform(_localTransform), model(_model)
    {
        model.transform = initialTransform;
    }
} // namespace sage
