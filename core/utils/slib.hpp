//
// Created by Steve Wheeler on 06/07/2024.
//

#pragma once

#include "common_types.hpp"
#include "raylib-cereal.hpp"
#include "raylib.h"

#include "entt/entt.hpp"
#include <string>

#include <optional>

namespace sage
{
    struct UberShaderComponent;

    class ImageSafe
    {
        Image image{};
        bool memorySafe = true;

      public:
        [[nodiscard]] const Image& GetImage();
        void SetImage(Image& _image);
        [[nodiscard]] Color GetColor(int x, int y) const;
        [[nodiscard]] bool HasLoaded() const;
        [[nodiscard]] int GetWidth() const;
        [[nodiscard]] int GetHeight() const;

        ImageSafe(const ImageSafe&) = delete;
        ImageSafe& operator=(const ImageSafe&) = delete;
        ImageSafe(ImageSafe&& other) noexcept;
        ImageSafe& operator=(ImageSafe&& other) noexcept;

        ~ImageSafe();
        explicit ImageSafe(Image _image, bool _memorySafe = true);
        explicit ImageSafe(const std::string& path, bool _memorySafe = true);
        explicit ImageSafe(bool _memorySafe = true);

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
        // TODO: Why don't glb ones have their keys set? Because they're deep copies?
        std::string modelKey{}; // The name/path of the model in the ResourceManager
        bool memorySafe = true;

        void UnloadShaderLocs() const;
        void UnloadMaterials() const;

      public:
        // TODO: return GetRlModel to const
        [[nodiscard]] Model& GetRlModel();
        [[nodiscard]] const Mesh& GetMesh(int num) const;
        [[nodiscard]] BoundingBox CalcLocalMeshBoundingBox(const Mesh& mesh, bool& success) const;
        [[nodiscard]] BoundingBox CalcLocalBoundingBox() const;
        [[nodiscard]] RayCollision GetRayMeshCollision(Ray ray, int meshNum, Matrix transform) const;
        void UpdateAnimation(ModelAnimation anim, int frame) const;
        void Draw(Vector3 position, float scale, Color tint) const;
        void Draw(Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) const;
        void DrawUber(
            UberShaderComponent* uber,
            Vector3 position,
            Vector3 rotationAxis,
            float rotationAngle,
            Vector3 scale,
            Color tint) const;
        [[nodiscard]] int GetMeshCount() const;
        [[nodiscard]] int GetMaterialCount() const;
        [[nodiscard]] Matrix GetTransform() const;
        void SetTransform(Matrix trans);
        void SetTexture(Texture texture, int materialIdx, MaterialMapIndex mapIdx) const;
        [[nodiscard]] Shader GetShader(int materialIdx) const;
        void SetShader(Shader shader, int materialIdx) const;
        void SetShader(Shader shader) const;
        void SetKey(const std::string& newKey);
        [[nodiscard]] const std::string& GetKey() const;
        ModelSafe(const ModelSafe&) = delete;
        ModelSafe& operator=(const ModelSafe&) = delete;
        ModelSafe(ModelSafe&& other) noexcept;
        ModelSafe& operator=(ModelSafe&& other) noexcept;
        explicit ModelSafe(const char* path, bool _memorySafe = true);
        explicit ModelSafe(Model& _model, bool _memorySafe = true);
        ModelSafe() = default;
        ~ModelSafe();

        friend class Renderable;
        friend class ResourceManager;
        friend class TextureTerrainOverlay;
        friend class UberShaderComponent;
        friend class UberShaderSystem;
        friend class RenderSystem;
    };
    bool AlmostEquals(Vector3 a, Vector3 b);
    bool PointInsideRect(Rectangle rec, Vector2 point);
    Vector2 Vec3ToVec2(const Vector3& vec3);
    Vector3 NegateVector(const Vector3& vec3);
    Vector3 Vector3MultiplyByValue(const Vector3& vec3, float value);
    Matrix ComposeMatrix(Vector3 translation, Quaternion rotation, Vector3 scale);
    int GetBoneIdByName(const BoneInfo* bones, int numBones, const char* boneName);
    Image GenImageGradientRadialTrans(int width, int height, float density, Color inner, Color outer);
    std::string StripPath(const std::string& fullPath);
} // namespace sage
