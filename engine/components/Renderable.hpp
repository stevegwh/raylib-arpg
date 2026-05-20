//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "engine/raylib-cereal.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/slib.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <cstdint>
#include <functional>
#include <string>
#include <variant>

namespace sage
{

    // Emplace this "tag" to draw this renderable "last" (or, at least, with the other deferred renderables)
    struct RenderableDeferred
    {
    };

    class Renderable
    {
        std::variant<std::monostate, ModelView, ModelMutable> model;
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

        // Returns the underlying view (ModelView pointer; also valid when holding a
        // ModelMutable, since it derives from ModelView). Returns nullptr if the
        // Renderable has no model (default-constructed / monostate).
        [[nodiscard]] ModelView* GetModel();
        [[nodiscard]] const ModelView* GetModel() const;

        // Returns a pointer to the mutable view if this Renderable holds one,
        // otherwise nullptr. Use when you need to call mutating methods
        // (SetTexture, SetMaterial) that don't exist on ModelView.
        [[nodiscard]] ModelMutable* GetMutable();
        [[nodiscard]] const ModelMutable* GetMutable() const;

        void SetModel(ModelView _model);
        void SetModel(ModelMutable _model);

        void Enable();
        void Disable();

        Renderable() = default;
        ~Renderable() = default;
        Renderable(const Renderable&) = default;
        Renderable& operator=(const Renderable&) = default;
        Renderable(Renderable&&) noexcept = default;
        Renderable& operator=(Renderable&&) noexcept = default;

        Renderable(ModelView _model, Matrix _localTransform);
        Renderable(ModelMutable _model, Matrix _localTransform);

        template <class Archive>
        void save(Archive& archive) const
        {
            std::uint8_t kind = 0;
            std::string key;
            if (const auto* mut = std::get_if<ModelMutable>(&model))
            {
                kind = 2;
                key = mut->GetKey();
            }
            else if (const auto* view = std::get_if<ModelView>(&model))
            {
                kind = 1;
                key = view->GetKey();
            }
            archive(kind, key, name, vanityName, initialTransform);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::uint8_t kind = 0;
            std::string key;
            archive(kind, key, name, vanityName, initialTransform);

            if (kind == 1)
            {
                ModelView view = ResourceManager::GetInstance().GetModelView(key);
                assert(view.GetRlModel().meshes != nullptr);
                view.SetTransform(initialTransform);
                model = std::move(view);
            }
            else if (kind == 2)
            {
                ModelMutable mut = ResourceManager::GetInstance().CreateModelMutable(key);
                assert(mut.GetRlModel().meshes != nullptr);
                mut.SetTransform(initialTransform);
                model = std::move(mut);
            }
            else
            {
                model = std::monostate{};
            }
        }

        template <class Inspector>
        void inspect(Inspector& i)
        {
            i.field("Active", active);
            i.field("Serializable", serializable);
            i.field("Name", name, false);
            i.field("Hint", hint);
        }
    };
} // namespace sage
