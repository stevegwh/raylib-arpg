//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ECSManager.hpp"
#include "Serializer.hpp"

namespace sage
{
ECSManager::ECSManager(entt::registry* _registry, UserInput* userInput) :
    registry(_registry),
    renderSystem(std::make_unique<RenderSystem>(_registry)),
    collisionSystem(std::make_unique<sage::CollisionSystem>(_registry)),
    transformSystem(std::make_unique<sage::TransformSystem>(_registry)),
    navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry)),
    animationSystem(std::make_unique<AnimationSystem>(_registry))
{
//    entt::entity rootNodeId = registry->create();
//    auto rootNodeObject = std::make_unique<WorldObject>();
//    worldSystem = std::make_unique<sage::WorldSystem>(_registry, rootNodeId);
//    registry->emplace<WorldObject>(rootNodeId, new WorldObject());
    actorMovementSystem = std::make_unique<sage::ActorMovementSystem>(_registry, userInput);
}

void ECSManager::DeserializeMap()
{
//    auto data = Serializer::DeserializeFile();
//    if (data.has_value())
//    {
//        for (const auto& entityIdEntry : data.value())
//        {
//            //const std::string& entityId = entityIdEntry.first;
//            auto newId = std::to_string(registry->create());
//            const auto& componentMap = entityIdEntry.second;
//
//            if (componentMap.find(transformSystem->getComponentName()) != componentMap.end())
//            {
//                const auto& transformComponent = componentMap.at(transformSystem->getComponentName());
//                transformSystem->DeserializeComponents(newId, transformComponent);
//            }
//
//            if (componentMap.find(renderSystem->getComponentName()) != componentMap.end())
//            {
//                const auto& renderableComponent = componentMap.at(renderSystem->getComponentName());
//                renderSystem->DeserializeComponents(newId, renderableComponent);
//            }
//
//            if (componentMap.find(collisionSystem->getComponentName()) != componentMap.end())
//            {
//                const auto& collideableComponent = componentMap.at(collisionSystem->getComponentName());
//                collisionSystem->DeserializeComponents(newId, collideableComponent);
//            }
//        }
//    }
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
