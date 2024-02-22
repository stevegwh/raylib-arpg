//
// Created by steve on 22/02/2024.
//

#pragma once

#include <vector>
#include <memory>

#include "Entity.hpp"

namespace sage
{
    class Registry
    {
        explicit Registry() = default;
        std::vector<std::unique_ptr<Entity>> entities;
    public:
        static Registry& GetInstance()
        {
            static Registry instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
            return instance;
        }
        Registry(Registry const&) = delete;
        void operator=(Registry const&)  = delete;

        EntityID CreateEntity()
        {
            auto entity = std::make_unique<Entity>();
            EntityID id = entity->entityId;
            entities.push_back(std::move(entity));
            return id;
        }

        void DeleteEntity(EntityID entityId)
        {
            entities.at(entityId)->deleted = true;
        }
    };
}

