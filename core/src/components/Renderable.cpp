#include "Renderable.hpp"
//
// Created by Steve Wheeler on 03/05/2024.
//

#include <memory>
#include <utility>

namespace sage
{

    ModelSafe* Renderable::GetModel() const
    {
        // assert(model != nullptr);
        return model.get();
    }

    Renderable::Renderable(std::unique_ptr<ModelSafe> _model, Matrix _localTransform)
        : initialTransform(_localTransform), model(std::move(_model))
    {
        model->SetTransform(_localTransform);
    }

    Renderable::Renderable(Model _model, Matrix _localTransform)
        : initialTransform(_localTransform), model(std::make_unique<ModelSafe>(_model))
    {
        model->SetTransform(_localTransform);
    }

    Renderable::Renderable(ModelSafe _model, Matrix _localTransform)
        : initialTransform(_localTransform), model(std::make_unique<ModelSafe>(std::move(_model)))
    {
        model->SetTransform(_localTransform);
    }
} // namespace sage
