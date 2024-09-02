//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "ResourceManager.hpp"
#include <slib.hpp>

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "raylib-cereal.hpp"
#include "raylib.h"
#include "raymath.h"
#include <entt/entt.hpp>

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sage
{
    // Emplace this "tag" to draw this renderable "last" (or, at least, with the other deferred renderables)
    struct RenderableDeferred
    {
    };

    struct Renderable
    {
        Color hint = WHITE;
        bool active = true;
        Matrix initialTransform{};
        MaterialPaths materials;
        std::optional<Shader> shader;
        std::unordered_map<std::string, int> shaderLocs;
        std::function<void(entt::entity)> reqShaderUpdate;
        std::string name = "Default";
        bool serializable = true;
        // void SetModel(SafeModel _model);
        [[nodiscard]] Model GetModel() const;

        Renderable() = default;
        Renderable(const Renderable&) = delete;
        Renderable& operator=(const Renderable&) = delete;
        Renderable(SafeModel _model, MaterialPaths _materials, Matrix _localTransform);
        Renderable(SafeModel _model, Matrix _localTransform);
        ~Renderable();

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(model->rlModel(), name, materials, initialTransform);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            Model _model;

            archive(_model, name, materials, initialTransform);

            char* _name = new char[this->name.size() + 1];
            _model.meshes[0].name = _name;
            _model.transform = initialTransform;

            if (FileExists(materials.diffuse.c_str()))
            {
                auto texture = LoadTextureFromImage(ResourceManager::GetInstance().ImageLoad(materials.diffuse));
                _model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
            }
            if (FileExists(materials.specular.c_str()))
            {
                auto texture = LoadTextureFromImage(ResourceManager::GetInstance().ImageLoad(materials.specular));
                _model.materials[0].maps[MATERIAL_MAP_SPECULAR].texture = texture;
            }
            if (FileExists(materials.normal.c_str()))
            {
                auto texture = LoadTextureFromImage(ResourceManager::GetInstance().ImageLoad(materials.normal));
                _model.materials[0].maps[MATERIAL_MAP_NORMAL].texture = texture;
            }

            model = std::make_unique<SafeModel>(_model);
        }

      private:
        std::unique_ptr<SafeModel> model;
    };
} // namespace sage
