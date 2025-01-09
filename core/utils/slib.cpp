//
// Created by Steve Wheeler on 06/07/2024.
//

#include "slib.hpp"

#include "components/UberShaderComponent.hpp"
#include "raymath.h"
#include "ResourceManager.hpp"

#include <cstring>
#include <vector>

namespace sage
{

    const Image& ImageSafe::GetImage()
    {
        return image;
    }

    void ImageSafe::SetImage(Image& _image)
    {
        image = _image;
    }

    Color ImageSafe::GetColor(int x, int y) const
    {
        return GetImageColor(image, x, y);
    }

    bool ImageSafe::HasLoaded() const
    {
        return image.data != nullptr;
    }

    int ImageSafe::GetWidth() const
    {
        return image.width;
    }

    int ImageSafe::GetHeight() const
    {
        return image.height;
    }

    ImageSafe::ImageSafe(ImageSafe&& other) noexcept : image(other.image)
    {
        // Reset the source object's model to prevent double deletion
        memorySafe = other.memorySafe;
        other.image = {};
    }

    ImageSafe& ImageSafe::operator=(ImageSafe&& other) noexcept
    {

        if (this != &other && memorySafe)
        {
            // Clean up existing resources
            UnloadImage(image);

            // Move resources from other
            image = other.image;
            memorySafe = other.memorySafe;

            // Reset the source object's model
            other.image = {};
        }
        return *this;
    }

    ImageSafe::~ImageSafe()
    {
        if (memorySafe)
        {
            UnloadImage(image);
        }
    }

    ImageSafe::ImageSafe(Image _image, bool _memorySafe) : image(_image), memorySafe(_memorySafe)
    {
    }

    ImageSafe::ImageSafe(const std::string& path, bool _memorySafe)
        : image(LoadImage(path.c_str())), memorySafe(_memorySafe)
    {
    }

    ImageSafe::ImageSafe(bool _memorySafe) : memorySafe(_memorySafe)
    {
    }

    Model& ModelSafe::GetRlModel()
    {
        return rlmodel;
    }

    const Mesh& ModelSafe::GetMesh(int num) const
    {
        assert(num < rlmodel.meshCount);
        return rlmodel.meshes[num];
    }

    BoundingBox ModelSafe::CalcLocalMeshBoundingBox(const Mesh& mesh, bool& success) const
    {
        success = true;
        std::vector<float> vertices(mesh.vertices, mesh.vertices + mesh.vertexCount * 3);

        BoundingBox bb;
        bb.min = {0, 0, 0};
        bb.max = {0, 0, 0};

        if (mesh.vertexCount < 3)
        {
            success = false;
            return bb;
        }

        {
            float x = vertices[0];
            float y = vertices[1];
            float z = vertices[2];

            Vector3 v = {x, y, z};
            v = Vector3Transform(v, rlmodel.transform);

            bb.min = bb.max = v;
        }

        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            float x = vertices[i];
            float y = vertices[i + 1];
            float z = vertices[i + 2];

            Vector3 v = {x, y, z};
            v = Vector3Transform(v, rlmodel.transform);

            bb.min.x = std::min(bb.min.x, v.x);
            bb.min.y = std::min(bb.min.y, v.y);
            bb.min.z = std::min(bb.min.z, v.z);

            bb.max.x = std::max(bb.max.x, v.x);
            bb.max.y = std::max(bb.max.y, v.y);
            bb.max.z = std::max(bb.max.z, v.z);
        }

        return bb;
    }

    BoundingBox ModelSafe::CalcLocalBoundingBox() const
    {
        assert(rlmodel.meshCount > 0);

        std::optional<BoundingBox> bb;

        for (size_t i = 0; i < rlmodel.meshCount; ++i)
        {
            bool success = false;
            auto currentBB = CalcLocalMeshBoundingBox(rlmodel.meshes[i], success);
            if (!success) continue;

            if (!bb.has_value())
            {
                bb = currentBB;
                continue;
            }

            bb->min = {
                std::min(bb->min.x, currentBB.min.x),
                std::min(bb->min.y, currentBB.min.y),
                std::min(bb->min.z, currentBB.min.z)};

            bb->max = {
                std::max(bb->max.x, currentBB.max.x),
                std::max(bb->max.y, currentBB.max.y),
                std::max(bb->max.z, currentBB.max.z)};
        }

        assert(bb.has_value());

        return *bb;
    }

    RayCollision ModelSafe::GetRayMeshCollision(Ray ray, int meshNum, Matrix transform) const
    {
        assert(meshNum < GetMeshCount());
        Matrix mat = MatrixMultiply(rlmodel.transform, transform);
        return GetRayCollisionMesh(ray, rlmodel.meshes[meshNum], mat);
    }

    void ModelSafe::UpdateAnimation(ModelAnimation anim, int frame) const
    {
        // UpdateModelAnimation(rlmodel, anim, frame);
        UpdateModelAnimationBones(rlmodel, anim, frame);
    }

    void ModelSafe::Draw(Vector3 position, float scale, Color tint) const
    {
        Draw(position, {0, 1, 0}, 0, {scale, scale, scale}, tint);
    }

    void ModelSafe::Draw(
        Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) const
    {
        DrawModelEx(rlmodel, position, rotationAxis, rotationAngle, scale, tint);
    }

    // Expects angle in degrees
    void ModelSafe::DrawUber(
        UberShaderComponent* uber,
        Vector3 position,
        Vector3 rotationAxis,
        float rotationAngle,
        Vector3 scale,
        Color tint) const
    {
        // NB: This is mainly copied from rmodels "DrawModelEx", but we have added "SetShaderLocs" per material and
        // emission stuff.

        // Calculate transformation matrix from function parameters
        // Get transform matrix (rotation -> scale -> translation)
        Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
        Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
        Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

        Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

        // Combine model transformation matrix (model.transform) with matrix generated by function parameters
        // (matTransform)
        auto model = rlmodel;
        model.transform = MatrixMultiply(model.transform, matTransform);

        for (int i = 0; i < model.meshCount; i++)
        {

            uber->SetShaderBools(model.meshMaterial[i]); // Set shader booleans per material.

            if (uber->HasFlag(model.meshMaterial[i], UberShaderComponent::EmissiveTexture))
            {
                auto emTex = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_EMISSION].texture;
                SetShaderValue(
                    model.materials[i].shader,
                    model.materials[model.meshMaterial[i]].shader.locs[SHADER_LOC_MAP_EMISSION],
                    &emTex,
                    SHADER_UNIFORM_SAMPLER2D);
            }
            if (uber->HasFlag(model.meshMaterial[i], UberShaderComponent::EmissiveCol))
            {
                auto emCol = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_EMISSION].color;

                float values[4] = {
                    static_cast<float>(emCol.r) / 255.0f,
                    static_cast<float>(emCol.g) / 255.0f,
                    static_cast<float>(emCol.b) / 255.0f,
                    static_cast<float>(emCol.a) / 255.0f};
                // model.materials[i].shader.locs[SHADER_LOC_COLOR_AMBIENT]
                SetShaderValue(model.materials[i].shader, uber->colEmissiveLoc, &values, SHADER_UNIFORM_VEC4);
            }

            Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

            auto colorTint = WHITE;
            colorTint.r = static_cast<unsigned char>((static_cast<int>(color.r) * static_cast<int>(tint.r)) / 255);
            colorTint.g = static_cast<unsigned char>((static_cast<int>(color.g) * static_cast<int>(tint.g)) / 255);
            colorTint.b = static_cast<unsigned char>((static_cast<int>(color.b) * static_cast<int>(tint.b)) / 255);
            colorTint.a = static_cast<unsigned char>((static_cast<int>(color.a) * static_cast<int>(tint.a)) / 255);

            model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
            DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
            model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
        }
    }

    int ModelSafe::GetMeshCount() const
    {
        return rlmodel.meshCount;
    }

    int ModelSafe::GetMaterialCount() const
    {
        return rlmodel.materialCount;
    }

    Matrix ModelSafe::GetTransform() const
    {
        return rlmodel.transform;
    }

    void ModelSafe::SetTransform(Matrix trans)
    {
        rlmodel.transform = trans;
    }

    void ModelSafe::SetTexture(Texture texture, int materialIdx, MaterialMapIndex mapIdx) const
    {
        rlmodel.materials[materialIdx].maps[mapIdx].texture = texture;
    }

    Shader ModelSafe::GetShader(int materialIdx) const
    {
        assert(materialIdx < rlmodel.materialCount);
        return rlmodel.materials[materialIdx].shader;
    }

    void ModelSafe::SetShader(Shader shader, int materialIdx) const
    {
        assert(materialIdx < rlmodel.materialCount);
        rlmodel.materials[materialIdx].shader = shader;
    }

    void ModelSafe::SetShader(Shader shader) const
    {
        for (int i = 0; i < rlmodel.materialCount; ++i)
        {
            SetShader(shader, i);
        }
    }

    void ModelSafe::SetKey(const std::string& newKey)
    {
        modelKey = newKey;
    }

    const std::string& ModelSafe::GetKey() const
    {
        return modelKey;
    }

    ModelSafe::ModelSafe(ModelSafe&& other) noexcept : rlmodel(other.rlmodel)
    {
        // Reset the source object's model to prevent double deletion
        memorySafe = other.memorySafe;
        modelKey = other.modelKey;
        other.rlmodel = {};
    }

    ModelSafe& ModelSafe::operator=(ModelSafe&& other) noexcept
    {
        if (this != &other && memorySafe)
        {
            // Clean up existing resources
            UnloadModel(rlmodel);

            // Move resources from other
            rlmodel = other.rlmodel;
            memorySafe = other.memorySafe;
            modelKey = other.modelKey;

            // Reset the source object's model
            other.rlmodel = {};
        }
        return *this;
    }

    // Only needed if deep copying model's shaders
    void ModelSafe::UnloadShaderLocs() const
    {
        // The shader program gets unloaded by the resource manager
        for (int i = 0; i < rlmodel.materialCount; i++)
        {
            RL_FREE(rlmodel.materials[i].shader.locs);
        }
    }

    void ModelSafe::UnloadMaterials() const
    {
        for (int i = 0; i < rlmodel.materialCount; ++i)
        {
            // Unload loaded texture maps (avoid unloading default texture, managed by raylib)
            if (rlmodel.materials[i].maps != nullptr)
            {
                for (int j = 0; j < MAX_MATERIAL_MAPS; j++)
                {
                    if (rlmodel.materials[i].maps[j].texture.id != rlGetTextureIdDefault())
                        rlUnloadTexture(rlmodel.materials[i].maps[j].texture.id);
                }
            }
        }
    }

    ModelSafe::~ModelSafe()
    {
        if (memorySafe)
        {
            // NB: Textures are currently shared between model copies (deep copies or not)
            // this->UnloadMaterials();
            UnloadModel(rlmodel);
        }
    }

    ModelSafe::ModelSafe(Model& _model, bool _memorySafe) : rlmodel(_model)
    {
        memorySafe = _memorySafe;
        if (_memorySafe)
        {
            _model = {};
        }
    }

    ModelSafe::ModelSafe(const char* path, bool _memorySafe) : rlmodel(LoadModel(path)), memorySafe(_memorySafe)
    {
    }

    std::string TitleCase(const std::string& A)
    {

        std::string B;

        int pos = 0;
        int pre_pos = 0;

        pos = A.find(' ', pre_pos);

        while (pos != std::string::npos)
        {
            std::string sub = "";

            sub = A.substr(pre_pos, (pos - pre_pos));

            if (pre_pos != pos)
            {
                sub = A.substr(pre_pos, (pos - pre_pos));
            }
            else
            {
                sub = A.substr(pre_pos, 1);
            }

            sub[0] = toupper(sub[0]);
            B += sub + A[pos];

            if (pos < (A.length() - 1))
            {
                pre_pos = (pos + 1);
            }
            else
            {
                pre_pos = pos;
                break;
            }

            pos = A.find(' ', pre_pos);
        }

        std::string sub = A.substr(pre_pos, std::string::npos);
        sub[0] = toupper(sub[0]);
        B += sub;

        return B;
    }

    bool AlmostEquals(Vector3 a, Vector3 b)
    {
        const std::tuple aInt = {static_cast<int>(a.x), static_cast<int>(a.z)};
        const std::tuple bInt = {static_cast<int>(b.x), static_cast<int>(b.z)};
        return aInt == bInt;
    }

    bool PointInsideRect(Rectangle rec, Vector2 point)
    {
        return point.x >= rec.x && point.x <= rec.x + rec.width && point.y >= rec.y &&
               point.y <= rec.y + rec.height;
    }

    Vector2 Vec3ToVec2(const Vector3& vec3)
    {
        return {vec3.x, vec3.z};
    }

    Vector3 NegateVector(const Vector3& vec3)
    {
        return {-vec3.x, -vec3.y, -vec3.z};
    }

    Vector3 Vector3MultiplyByValue(const Vector3& vec3, float value)
    {
        return {vec3.x * value, vec3.y * value, vec3.z * value};
    }

    Matrix ComposeMatrix(Vector3 translation, Quaternion rotation, Vector3 scale)
    {
        Matrix matTranslation = MatrixTranslate(translation.x, translation.y, translation.z);
        Matrix matRotation = QuaternionToMatrix(rotation);
        Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);

        // Apply transformations in order: Scale -> Rotate -> Translate
        Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

        return matTransform;
    }

    int GetBoneIdByName(const BoneInfo* bones, int numBones, const char* boneName)
    {
        for (int i = 0; i < numBones; ++i)
        {
            if (strcmp(bones[i].name, boneName) == 0)
            {
                return i; // Found the bone, return its index
            }
        }
        return -1; // Bone not found
    }

    /**
     * Generates a gradient with transperency (raylib version does not have transparency)
     */
    Image GenImageGradientRadialTrans(int width, int height, float density, Color inner, Color outer)
    {
        Color* pixels = (Color*)RL_MALLOC(width * height * sizeof(Color));
        float radius = (width < height) ? (float)width / 2.0f : (float)height / 2.0f;

        float centerX = (float)width / 2.0f;
        float centerY = (float)height / 2.0f;

        // Set outer color's alpha to 0 (fully transparent)
        outer.a = 0;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float dist = hypotf((float)x - centerX, (float)y - centerY);
                float factor = (dist - radius * density) / (radius * (1.0f - density));

                factor = (float)fmax(factor, 0.0f);
                factor = (float)fmin(factor, 1.f);

                // Calculate alpha first
                unsigned char alpha = (unsigned char)((float)outer.a * factor + (float)inner.a * (1.0f - factor));

                // Only set color if alpha is not zero
                if (alpha > 0)
                {
                    pixels[y * width + x].r =
                        (unsigned char)((float)outer.r * factor + (float)inner.r * (1.0f - factor));
                    pixels[y * width + x].g =
                        (unsigned char)((float)outer.g * factor + (float)inner.g * (1.0f - factor));
                    pixels[y * width + x].b =
                        (unsigned char)((float)outer.b * factor + (float)inner.b * (1.0f - factor));
                    pixels[y * width + x].a = alpha;
                }
                else
                {
                    pixels[y * width + x] = Color{0, 0, 0, 0}; // Fully transparent
                }
            }
        }

        Image image = {
            .data = pixels,
            .width = width,
            .height = height,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8

        };

        return image;
    }

    std::string StripPath(const std::string& fullPath)
    {
        if (fullPath.empty()) return "";
        size_t lastSlash = fullPath.find_last_of("/\\");

        size_t lastDot = fullPath.find_last_of('.');

        size_t startPos = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
        size_t length = (lastDot == std::string::npos) ? std::string::npos : lastDot - startPos;

        return fullPath.substr(startPos, length);
    }

} // namespace sage