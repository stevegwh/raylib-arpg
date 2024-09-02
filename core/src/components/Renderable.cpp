#include "Renderable.hpp"
//
// Created by Steve Wheeler on 03/05/2024.
//

#include "Renderable.hpp"

#include <memory>

namespace sage
{

    // void Renderable::SetModel(SafeModel _model)
    // {
    //     model = std::make_unique<SafeModel>(std::move(_model));
    // }

    Model Renderable::GetModel() const
    {
        // assert(model != nullptr);
        return model->rlModel();
    }

    Renderable::~Renderable()
    {
        if (shader.has_value())
        {
            UnloadShader(shader.value());
        }
    }

    Renderable::Renderable(SafeModel _model, MaterialPaths _materials, Matrix _localTransform)
        : initialTransform(_localTransform),
          materials(std::move(_materials)),
          model(std::make_unique<SafeModel>(std::move(_model)))
    {
        model->rlModel().transform = initialTransform;
    }

    Renderable::Renderable(SafeModel _model, Matrix _localTransform)
        : initialTransform(_localTransform), model(std::make_unique<SafeModel>(std::move(_model)))
    {
        model->rlModel().transform = initialTransform;
    }
} // namespace sage
