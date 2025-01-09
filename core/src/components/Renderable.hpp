//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "raylib-cereal.hpp"
#include "raylib.h"
#include "ResourceManager.hpp"
#include "slib.hpp"

#include "entt/entt.hpp"

#include <functional>
#include <string>
#include <vector>

namespace sage
{

    // Emplace this "tag" to draw this renderable "last" (or, at least, with the other deferred renderables)
    struct RenderableDeferred
    {
    };

    class Renderable
    {
        std::unique_ptr<ModelSafe> model;
        std::string name = "Default";
        std::string vanityName;
        void setVanityName();

      public:
        Color hint = WHITE;
        bool active = true;
        Matrix initialTransform{};
        std::function<void(entt::entity)> reqShaderUpdate;
        bool serializable = true;

        [[nodiscard]] const std::string& GetName() const;
        void SetName(const std::string& _name);
        [[nodiscard]] std::string GetVanityName() const;
        [[nodiscard]] ModelSafe* GetModel() const;

        void Enable();
        void Disable();

        Renderable() = default;
        Renderable(const Renderable&) = delete;
        Renderable& operator=(const Renderable&) = delete;
        Renderable(std::unique_ptr<ModelSafe> _model, Matrix _localTransform);
        Renderable(Model _model, Matrix _localTransform);
        Renderable(ModelSafe _model, Matrix _localTransform);

        template <class Archive>
        void save(Archive& archive) const
        {
            // assert(!model->GetKey().empty());
            archive(model->GetKey(), name, vanityName, initialTransform);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::string modelKey;
            archive(modelKey, name, vanityName, initialTransform);
            ModelSafe modelSafe(ResourceManager::GetInstance().GetModelCopy(modelKey));
            // Model data must be deserialised from ResourceManager before deserialising models
            assert(modelSafe.rlmodel.meshes != nullptr);
            modelSafe.rlmodel.transform = initialTransform;
            model = std::make_unique<ModelSafe>(std::move(modelSafe));
        }
    };
} // namespace sage
