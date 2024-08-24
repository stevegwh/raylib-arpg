#pragma once
#include <unordered_map>
#include <memory>
#include <typeindex>
#include "EntityEventBridge.hpp"

namespace sage
{

class EntityEventBridgeManager
{
private:
    using BridgePtr = std::unique_ptr<void, void(*)(void*)>;
    std::unordered_map<entt::entity, std::unordered_map<std::type_index, BridgePtr>> bridges;

    template<typename... Args>
    static void deleteBridge(void* ptr)
    {
        delete static_cast<EntityEventBridge<Args...>*>(ptr);
    }

public:
    template<typename... Args>
    void CreateBridge(entt::entity entity, entt::sigh<void(Args...)>& inSignal, entt::sigh<void(entt::entity, Args...)>& outSignal)
    {
        auto bridge = std::make_unique<EntityEventBridge<Args...>>(entity);
        bridge->BridgeEvents(inSignal, outSignal);

        BridgePtr bridgePtr(bridge.release(), &EntityEventBridgeManager::deleteBridge<Args...>);
        bridges[entity][std::type_index(typeid(EntityEventBridge<Args...>))] = std::move(bridgePtr);
    }

    template<typename... Args>
    void RemoveBridge(entt::entity entity)
    {
        auto entityIt = bridges.find(entity);
        if (entityIt != bridges.end())
        {
            auto& entityBridges = entityIt->second;
            auto bridgeIt = entityBridges.find(std::type_index(typeid(EntityEventBridge<Args...>)));
            if (bridgeIt != entityBridges.end())
            {
                entityBridges.erase(bridgeIt);
            }
            if (entityBridges.empty())
            {
                bridges.erase(entityIt);
            }
        }
    }

    void RemoveAllBridges(entt::entity entity)
    {
        bridges.erase(entity);
    }

    void Clear()
    {
        bridges.clear();
    }
};

} // namespace sage