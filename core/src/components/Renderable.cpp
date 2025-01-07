#include "Renderable.hpp"
//
// Created by Steve Wheeler on 03/05/2024.
//

#include <algorithm>

namespace sage
{

    const std::string& Renderable::GetName() const
    {
        return name;
    }

    void Renderable::SetName(const std::string& _name)
    {
        name = _name;
    }

    std::string Renderable::GetVanityName() const
    {
        auto vanity = name;
        std::replace(vanity.begin(), vanity.end(), '_', ' ');
        return vanity;
    }

    ModelSafe* Renderable::GetModel() const
    {
        // assert(model != nullptr);
        return model.get();
    }

    void Renderable::Enable()
    {
        active = true;
    }

    void Renderable::Disable()
    {
        active = false;
    }

    Renderable::Renderable(std::unique_ptr<ModelSafe> _model, Matrix _localTransform)
        : model(std::move(_model)), initialTransform(_localTransform)
    {
        model->SetTransform(_localTransform);
    }

    Renderable::Renderable(Model _model, Matrix _localTransform)
        : model(std::make_unique<ModelSafe>(_model)), initialTransform(_localTransform)
    {
        model->SetTransform(_localTransform);
    }

    Renderable::Renderable(ModelSafe _model, Matrix _localTransform)
        : model(std::make_unique<ModelSafe>(std::move(_model))), initialTransform(_localTransform)
    {
        model->SetTransform(_localTransform);
    }
} // namespace sage
