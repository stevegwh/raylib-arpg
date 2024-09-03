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

    // Shader SafeModel::GetShader()
    // {
    //     return model.materials[MATERIAL_MAP_DIFFUSE].shader;
    // }

    Model& SafeModel::rlModel()
    {
        return model;
    }

    SafeModel::SafeModel(SafeModel&& other) noexcept : model(other.model)
    {
        // Reset the source object's model to prevent double deletion
        other.model = {};
    }

    SafeModel& SafeModel::operator=(SafeModel&& other) noexcept
    {
        if (this != &other)
        {
            // Clean up existing resources
            UnloadModel(model);

            // Move resources from other
            model = other.model;

            // Reset the source object's model
            other.model = {};
        }
        return *this;
    }

    // void SafeModel::SetModel(Model& _model)
    // {
    //     UnloadModel(model);
    //     model = _model;
    // }

    SafeModel::~SafeModel()
    {
        UnloadModel(model);
    }

    SafeModel::SafeModel(Model _model) : model(_model)
    {
        // Should deep copy Model and unload the old one?
    }

    SafeModel::SafeModel(const char* path)
    {
        model = LoadModel(path);
    }

    Vector2 Vec3ToVec2(const Vector3& vec3)
    {
        return {vec3.x, vec3.z};
    }

    BoundingBox CalculateModelBoundingBox(const Model& model)
    {
        Mesh mesh = model.meshes[0];
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
            // Assuming rl.Vector3Transform is a function that transforms a Vector3
            // using the given transform.
            v = Vector3Transform(v, model.transform);

            bb.min = bb.max = v;
        }

        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            float x = vertices[i];
            float y = vertices[i + 1];
            float z = vertices[i + 2];

            Vector3 v = {x, y, z};
            v = Vector3Transform(v, model.transform);

            bb.min.x = std::min(bb.min.x, v.x);
            bb.min.y = std::min(bb.min.y, v.y);
            bb.min.z = std::min(bb.min.z, v.z);

            bb.max.x = std::max(bb.max.x, v.x);
            bb.max.y = std::max(bb.max.y, v.y);
            bb.max.z = std::max(bb.max.z, v.z);
        }

        return bb;
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