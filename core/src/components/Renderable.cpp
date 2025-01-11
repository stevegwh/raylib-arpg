#include "Renderable.hpp"
//
// Created by Steve Wheeler on 03/05/2024.
//

#include "ParsingHelpers.hpp"
#include "slib.hpp"
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
        setVanityName();
    }

    void Renderable::setVanityName()
    {
        std::string vanity = name;

        if (name[0] == '_') // Remove tag
        {
            if (const auto endPos = name.substr(1).find_first_of('_'); endPos != std::string::npos)
            {
                vanity = name.substr(endPos + 1);
            }
        }

        std::ranges::replace(vanity, '_', ' ');

        vanity = parsing::trim(vanity);

        vanityName = TitleCase(vanity);
    }

    std::string Renderable::GetVanityName() const
    {
        return vanityName;
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
