//
// Created by Steve Wheeler on 06/07/2024.
//

#pragma once

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
        // If Image is a deepCopy, this class will use RAII.
        bool deepCopy = true;

      public:
        [[nodiscard]] const Image& GetImage() const;
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
        explicit ImageSafe(const Image& _image, bool _deepCopy = true);
        explicit ImageSafe(const std::string& path, bool _deepCopy = true);
        explicit ImageSafe(bool _deepCopy = true);

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(image);
        };
    };

    class ResourceManager;

    /**
     * Non-owning, copyable view of a Model stored in ResourceManager. Read-only public
     * API — the public surface intentionally exposes no material/texture/shader mutators
     * because the underlying entry may be shared across many viewers. Lifetime is
     * controlled by ResourceManager (scene-tied); a view never releases storage.
     */
    class ModelView
    {
      protected:
        Model rlmodel{};
        std::string assetKey{};

      public:
        [[nodiscard]] const Model& GetRlModel() const;
        [[nodiscard]] const Mesh& GetMesh(int num) const;
        [[nodiscard]] BoundingBox CalcLocalMeshBoundingBox(const Mesh& mesh, bool& success) const;
        [[nodiscard]] BoundingBox CalcLocalBoundingBox() const;
        [[nodiscard]] RayCollision GetRayMeshCollision(const Ray& ray, int meshNum, const Matrix& transform) const;
        void UpdateAnimation(const ModelAnimation& anim, unsigned int frame) const;
        void Draw(const Vector3& position, float scale, const Color& tint) const;
        void Draw(
            const Vector3& position,
            const Vector3& rotationAxis,
            const float rotationAngle,
            const Vector3& scale,
            const Color& tint) const;
        void DrawUber(
            UberShaderComponent* uber,
            const Vector3& position,
            const Vector3& rotationAxis,
            const float rotationAngle,
            const Vector3& scale,
            const Color& tint) const;
        [[nodiscard]] int GetMeshCount() const;
        [[nodiscard]] int GetMaterialCount() const;
        [[nodiscard]] Matrix GetTransform() const;
        // Mutates this view's local copy of Model::transform only; not propagated to any
        // other view of the same entry. Use for per-entity local transforms.
        void SetTransform(Matrix trans);
        [[nodiscard]] Shader GetShader(int materialIdx) const;
        // Mutates the shader on this view's underlying entry. If the entry is shared
        // (this is a plain ModelView, not the deep-copy backing of a ModelMutable),
        // the change is visible to all viewers of the same entry — by design, used
        // for system-wide shader assignments (LightManager, UberShaderSystem, etc.).
        void SetShader(Shader shader, int materialIdx) const;
        void SetShader(Shader shader) const;
        [[nodiscard]] const std::string& GetKey() const;

        ModelView() = default;
        ~ModelView() = default;
        ModelView(const ModelView&) = default;
        ModelView& operator=(const ModelView&) = default;
        ModelView(ModelView&&) noexcept = default;
        ModelView& operator=(ModelView&&) noexcept = default;

        friend class ResourceManager;
    };

    /**
     * Non-owning, copyable view of a Model in ResourceManager's mutable pool. Each
     * entry in that pool is a deep copy with private materials, so material/texture/
     * shader mutations are local to the entry. Copying a ModelMutable yields another
     * view onto the same entry — both copies see the same mutations.
     *
     * Mutators (SetMaterial / SetTexture) write through the internal materials pointer,
     * they do not reallocate it. A future mutator must preserve that invariant or
     * sibling copies will diverge from RM's record.
     */
    class ModelMutable : public ModelView
    {
        std::string instanceKey{};

      public:
        using ModelView::ModelView;
        using ModelView::SetShader;

        void SetTexture(Texture texture, int materialIdx, MaterialMapIndex mapIdx) const;
        void SetMaterial(unsigned int idx, Material mat) const;

        // Mutable handle to the underlying raylib Model. Use for direct buffer updates
        // (e.g. UpdateMeshBuffer on a procedurally-evolving mesh). The pointers inside
        // (meshes/materials/etc.) are shared with the RM-stored entry — writes through
        // them reach the entry. Do NOT reassign those pointers; that would diverge this
        // view from the RM record.
        [[nodiscard]] Model& GetRlModelMut();

        [[nodiscard]] const std::string& GetInstanceKey() const;

        friend class ResourceManager;
    };

    // Frees model meshes/bones/bindPose without unloading individual materials.
    // Use when materials are shared (e.g. owned by ResourceManager::materialMap).
    void sgUnloadModel(const Model& model);

    std::string TitleCase(const std::string& A);
    bool AlmostEquals(Vector3 a, Vector3 b);
    bool PointInsideRect(Rectangle rec, Vector2 point);
    Vector2 Vec3ToVec2(const Vector3& vec3);
    Vector3 NegateVector(const Vector3& vec3);
    Vector3 Vector3MultiplyByValue(const Vector3& vec3, float value);
    Vector2 Vector2MultiplyByValue(const Vector2& vec3, float value);
    Matrix ComposeMatrix(Vector3 translation, Quaternion rotation, Vector3 scale);
    int GetBoneIdByName(const BoneInfo* bones, int numBones, const char* boneName);
    Image GenImageGradientRadialTrans(int width, int height, float density, Color inner, Color outer);
    std::string StripPath(const std::string& fullPath);
} // namespace sage
