//
// Created by steve on 22/02/2024.
//

#pragma once

#include <vector>
#include <unordered_map>
#include <memory>


#include "Entity.hpp"

namespace sage
{
    class Registry
    {
        explicit Registry() = default;
        std::unordered_map<EntityID, std::unique_ptr<Entity>> entities; // TODO: Change to vector and entityId can be an index
        std::vector<EntityID> stagedForDeletion;
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
            entities.emplace(id, std::move(entity));
            return id;
        }
        
        const Entity* GetEntity(EntityID id)
        {
            return entities.at(id).get();
        }

        // Stages an entity for deletion (does not immediately remove entity)
        void DeleteEntity(EntityID entityId)
        {
            stagedForDeletion.push_back(entityId);
        }
        
        // Should run at end of update loop
        void RunMaintainance()
        {
            if (stagedForDeletion.empty()) return;
            for (auto id: stagedForDeletion) 
            {
                entities.erase(id); // Destructor of Entity will be called and trigger OnDelete event for all components
            }
            stagedForDeletion.clear();
        }
    };
}

