//
// Created by Steve Wheeler on 20/02/2024.
//

#include "Scene.hpp"

namespace sage
{
    void Scene::AddGameObject(sage::GameObject* gameObject)
    {
        gameObjects.push_back(gameObject);
    }
    
    void Scene::Draw()
    {
        for (const auto& go : gameObjects) 
        {
            go->GetRenderable()->Draw();
        }
    }
    
    Scene::~Scene()
    {
        for (auto go: gameObjects) 
        {
            delete go;
        }
    }
}
