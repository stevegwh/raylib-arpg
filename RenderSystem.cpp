//
// Created by Steve Wheeler on 21/02/2024.
//

#include "RenderSystem.hpp"
#include "raylib.h"
#include "GameManager.hpp"

namespace sage
{
RenderSystem::~RenderSystem()
{
    

}

void RenderSystem::Update()
{
    for (const auto& renderable : components)
    {
        if (renderable.second->anim && GetFrameTime() > 1)
        {
            renderable.second->animIndex = (renderable.second->animIndex + 1)%renderable.second->animsCount;
            // Update model animation
            ModelAnimation anim = renderable.second->animation[renderable.second->animIndex];
            renderable.second->animCurrentFrame = (renderable.second->animCurrentFrame + 1) % anim.frameCount;
            UpdateModelAnimation(renderable.second->model, anim, renderable.second->animCurrentFrame);
        }
    }
}

void RenderSystem::Draw() const
{
    for (const auto& renderable : components)
    {
        if (renderable.second->anim)
        {
            // Update model animation
            ModelAnimation anim = renderable.second->animation[renderable.second->animIndex];
            renderable.second->animCurrentFrame = (renderable.second->animCurrentFrame + 1) % anim.frameCount;
            UpdateModelAnimation(renderable.second->model, anim, renderable.second->animCurrentFrame);

            renderable.second->model.transform = renderable.second->initialTransform;
        }
        
        DrawModel(renderable.second->model, renderable.second->position, renderable.second->scale, WHITE);
    }
}

void RenderSystem::DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, std::string>& data)
{
    int id = std::stoi(entityId);
    
    sage::Material mat = { .diffuse = LoadTexture(data.at("Material").c_str()), .path = data.at("Material") };
    
    std::string modelPath = data.at("Model");
    Model model = LoadModel(modelPath.c_str());
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
    
    auto renderable = std::make_unique<Renderable>(id, model, mat, modelPath, MatrixIdentity());
    auto t = ECS->transformSystem->GetComponent(id);
    AddComponent(std::move(renderable), t);
}

BoundingBox RenderSystem::GetModelBoundingBox(EntityID id) 
{
    auto model = components.at(id)->model;
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

    for (size_t i = 0; i < vertices.size(); i += 3) {
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

void RenderSystem::onTransformUpdate(EntityID id)
{
    auto t = ECS->transformSystem->GetComponent(id);
    auto r = components.at(id).get();
    r->position = t->position;
    r->scale = t->scale;
    r->rotation = t->rotation;
}

void RenderSystem::AddComponent(std::unique_ptr<Renderable> component, const sage::Transform* const transform)
{
    eventManager->Subscribe([p = this, id = component->entityId]
                           { p->onTransformUpdate(id); }, *transform->OnPositionUpdate);
    
    component->position = transform->position;
    component->scale = transform->scale;

    m_addComponent(std::move(component));
}

}

