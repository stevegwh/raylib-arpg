//
// Created by Steve Wheeler on 06/07/2024.
//

#pragma once

#include "raylib-cereal.hpp"
#include "raylib.h"

#include <entt/entt.hpp>
#include <string>

namespace sage
{
    class ImageSafe
    {
        Image image{};

      public:
        const Image& GetImage();
        void SetImage(Image& _image);
        [[nodiscard]] Color GetColor(int x, int y) const;
        [[nodiscard]] bool HasLoaded() const;
        [[nodiscard]] int GetWidth() const;
        [[nodiscard]] int GetHeight() const;
        ~ImageSafe();
        explicit ImageSafe(const std::string& path);
        ImageSafe() = default;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(image);
        };
    };

    // Allowed unsafe access to model
    class Renderable;            // Forward dec for friend (Needed for serialisation)
    class ResourceManager;       // Forward dec for friend (Needs friend due to deep copy (could move that here))
    class TextureTerrainOverlay; // Forward dec for friend (Changes mesh data on the fly)

    /**
     * Defines a memory safe wrapper for raylib model.
     * Set "instanced" to true to disable memory management.
     */
    class ModelSafe
    {
        Model rlmodel{};
        std::string modelKey; // The name of the model in the ResourceManager
        bool instanced = false;

        void UnloadShaderLocs() const;
        void UnloadMaterials() const;

      public:
        [[nodiscard]] const Model& GetRlModel() const;
        [[nodiscard]] BoundingBox CalcLocalBoundingBox() const;
        [[nodiscard]] RayCollision GetRayMeshCollision(Ray ray, int meshNum, Matrix transform) const;
        void UpdateAnimation(ModelAnimation anim, int frame);
        void Draw(Vector3 position, float scale, Color tint);
        void Draw(Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);
        [[nodiscard]] int GetMeshCount() const;
        [[nodiscard]] int GetMaterialCount() const;
        [[nodiscard]] Matrix GetTransform() const;
        void SetTransform(Matrix trans);
        void SetTexture(Texture texture, int materialIdx, MaterialMapIndex mapIdx) const;
        [[nodiscard]] Shader GetShader(int materialIdx) const;
        void SetShader(Shader shader, int materialIdx) const;
        void SetKey(const std::string& newKey);
        [[nodiscard]] std::string GetKey() const;
        ModelSafe(const ModelSafe&) = delete;
        ModelSafe& operator=(const ModelSafe&) = delete;
        ModelSafe(ModelSafe&& other) noexcept;
        ModelSafe& operator=(ModelSafe&& other) noexcept;
        explicit ModelSafe(const char* path, bool _instanced = false);
        explicit ModelSafe(Model& _model, bool _instanced = false);
        ModelSafe() = default;
        ~ModelSafe();

        friend class Renderable;
        friend class ResourceManager;
        friend class TextureTerrainOverlay;
    };

    Vector2 Vec3ToVec2(const Vector3& vec3);
    Vector3 NegateVector(const Vector3& vec3);
    Vector3 Vector3MultiplyByValue(const Vector3& vec3, float value);
    Image GenImageGradientRadialTrans(int width, int height, float density, Color inner, Color outer);
} // namespace sage
