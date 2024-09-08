//
// Created by Steve Wheeler on 06/07/2024.
//

#include "slib.hpp"

#include "raymath.h"
#include "ResourceManager.hpp"

#include <cstring>
#include <vector>

namespace sage
{
    const Model& ModelSafe::GetRlModel() const
    {
        return rlmodel;
    }

    BoundingBox ModelSafe::CalcLocalBoundingBox() const
    {
        Mesh mesh = rlmodel.meshes[0];
        std::vector<float> vertices(mesh.vertexCount * 3);
        memcpy(&vertices[0], mesh.vertices, sizeof(float) * mesh.vertexCount * 3);

        BoundingBox bb;
        bb.min = {0, 0, 0};
        bb.max = {0, 0, 0};

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

    const Image& ImageSafe::GetImage()
    {
        return image;
    }

    void ImageSafe::SetImage(Image& _image)
    {
        image = _image;
        _image = {};
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

    ImageSafe::~ImageSafe()
    {
        UnloadImage(image);
    }

    ImageSafe::ImageSafe(const std::string& path) : image(LoadImage(path.c_str()))
    {
    }

    RayCollision ModelSafe::GetRayMeshCollision(Ray ray, int meshNum, Matrix transform) const
    {
        assert(meshNum < GetMeshCount());
        Matrix mat = MatrixMultiply(rlmodel.transform, transform);
        return GetRayCollisionMesh(ray, rlmodel.meshes[meshNum], mat);
    }

    void ModelSafe::UpdateAnimation(ModelAnimation anim, int frame)
    {
        UpdateModelAnimation(rlmodel, anim, frame);
    }

    void ModelSafe::Draw(Vector3 position, float scale, Color tint)
    {
        DrawModel(rlmodel, position, scale, tint);
    }

    void ModelSafe::Draw(Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
    {
        DrawModelEx(rlmodel, position, rotationAxis, rotationAngle, scale, tint);
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

    ModelSafe::ModelSafe(ModelSafe&& other) noexcept : rlmodel(other.rlmodel)
    {
        // Reset the source object's model to prevent double deletion
        instanced = other.instanced;
        other.rlmodel = {};
    }

    ModelSafe& ModelSafe::operator=(ModelSafe&& other) noexcept
    {
        if (this != &other)
        {
            // Clean up existing resources
            UnloadModel(rlmodel);

            // Move resources from other
            rlmodel = other.rlmodel;
            instanced = other.instanced;

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

    void ModelSafe::UnloadModelTextures() const
    {
        // Texture data managed by resource manager
        for (int i = 0; i < rlmodel.meshCount; i++)
        {
            if (rlmodel.meshes[i].texcoords)
            {
                RL_FREE(rlmodel.meshes[i].texcoords);
            }
            if (rlmodel.meshes[i].texcoords2)
            {
                RL_FREE(rlmodel.meshes[i].texcoords2);
            }
        }
    }

    ModelSafe::~ModelSafe()
    {
        if (!instanced)
        {
            UnloadModel(rlmodel);
        }
    }

    ModelSafe::ModelSafe(Model& _model, bool _instanced) : rlmodel(_model)
    {
        instanced = _instanced;
        if (!_instanced)
        {
            _model = {};
        }
    }

    ModelSafe::ModelSafe(const char* path, bool _instanced) : rlmodel(LoadModel(path)), instanced(_instanced)
    {
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

} // namespace sage