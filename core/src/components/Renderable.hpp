//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "ResourceManager.hpp"
#include <slib.hpp>

#include "../../../../../../../Library/Developer/CommandLineTools/SDKs/MacOSX14.4.sdk/System/Library/Frameworks/CoreGraphics.framework/Headers/CGPDFDictionary.h"
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

    class Renderable
    {
        std::unique_ptr<ModelSafe> model;

      public:
        Color hint = WHITE;
        bool active = true;
        Matrix initialTransform{};
        MaterialPaths materials;
        std::function<void(entt::entity)> reqShaderUpdate;
        std::string name = "Default";
        bool serializable = true;
        [[nodiscard]] ModelSafe* GetModel() const;

        Renderable() = default;
        Renderable(const Renderable&) = delete;
        Renderable& operator=(const Renderable&) = delete;
        Renderable(std::unique_ptr<ModelSafe> _model, MaterialPaths _materials, Matrix _localTransform);
        Renderable(std::unique_ptr<ModelSafe> _model, Matrix _localTransform);
        Renderable(Model _model, MaterialPaths _materials, Matrix _localTransform);
        Renderable(Model _model, Matrix _localTransform);
        Renderable(ModelSafe _model, MaterialPaths _materials, Matrix _localTransform);
        Renderable(ModelSafe _model, Matrix _localTransform);

        template <class Archive>
        void save(Archive& archive) const
        {
            assert(!model->GetKey().empty());
            archive(model->GetKey(), name, materials, initialTransform);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::string modelKey;

            archive(modelKey, name, materials, initialTransform);

            ModelSafe modelSafe(ResourceManager::GetInstance().LoadModelCopy(modelKey));

            // Model data must be deserialised from ResourceManager before deserialising models
            assert(modelSafe.rlmodel.meshes != nullptr);

            modelSafe.SetKey(modelKey);

            modelSafe.rlmodel.transform = initialTransform;

            if (FileExists(materials.diffuse.c_str()))
            {
                modelSafe.rlmodel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
                    ResourceManager::GetInstance().TextureLoad(materials.diffuse);
            }
            if (FileExists(materials.specular.c_str()))
            {
                modelSafe.rlmodel.materials[0].maps[MATERIAL_MAP_SPECULAR].texture =
                    ResourceManager::GetInstance().TextureLoad(materials.specular);
            }
            if (FileExists(materials.normal.c_str()))
            {
                modelSafe.rlmodel.materials[0].maps[MATERIAL_MAP_NORMAL].texture =
                    ResourceManager::GetInstance().TextureLoad(materials.normal);
            }

            model = std::make_unique<ModelSafe>(std::move(modelSafe));
        }
    };
} // namespace sage
