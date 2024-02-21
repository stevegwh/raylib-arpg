//
// Created by Steve Wheeler on 20/02/2024.
//

#pragma once
#include <vector>

#include "GameObject.hpp"

namespace sage
{
    class Scene
    {
        std::vector<GameObject*> gameObjects; // TODO: Need a concept of a graph
    public:
        Scene() : gameObjects() {}
        ~Scene();
        void Draw();
        void AddGameObject(GameObject* gameObject);
    };
}

