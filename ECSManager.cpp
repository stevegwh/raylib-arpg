//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ECSManager.hpp"

#include "Serializer.hpp"

namespace sage
{
ECSManager::ECSManager(UserInput* userInput) :
    renderSystem(std::make_unique<RenderSystem>()),
    collisionSystem(std::make_unique<sage::CollisionSystem>()),
    transformSystem(std::make_unique<sage::TransformSystem>()),
    navigationGridSystem(std::make_unique<NavigationGridSystem>())
{
    EntityID rootNodeId = Registry::GetInstance().CreateEntity();
    auto rootNodeObject = std::make_unique<WorldObject>(rootNodeId);
    worldSystem = std::make_unique<sage::WorldSystem>(rootNodeId);
    worldSystem->AddComponent(std::move(rootNodeObject));
    actorMovementSystem = std::make_unique<sage::ActorMovementSystem>(userInput);
}

void ECSManager::DeserializeMap()
{
    auto data = Serializer::DeserializeFile();
    if (data.has_value())
    {
        for (const auto& entityIdEntry : data.value())
        {
            //const std::string& entityId = entityIdEntry.first;
            auto newId = std::to_string(Registry::GetInstance().CreateEntity());
            const auto& componentMap = entityIdEntry.second;

            if (componentMap.find(transformSystem->getComponentName()) != componentMap.end())
            {
                const auto& transformComponent = componentMap.at(transformSystem->getComponentName());
                transformSystem->DeserializeComponents(newId, transformComponent);
            }

            if (componentMap.find(renderSystem->getComponentName()) != componentMap.end())
            {
                const auto& renderableComponent = componentMap.at(renderSystem->getComponentName());
                renderSystem->DeserializeComponents(newId, renderableComponent);
            }

            if (componentMap.find(collisionSystem->getComponentName()) != componentMap.end())
            {
                const auto& collideableComponent = componentMap.at(collisionSystem->getComponentName());
                collisionSystem->DeserializeComponents(newId, collideableComponent);
            }
        }
    }
}

void ECSManager::SerializeMap() const
{
    SerializationData serializeData;
    transformSystem->SerializeComponents(serializeData);
    renderSystem->SerializeComponents(serializeData);
    collisionSystem->SerializeComponents(serializeData);

    Serializer::SerializeToFile(serializeData);
}
}
