//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <memory>

#include "Event.hpp"

namespace sage
{
    typedef int EntityID;
    
    struct Entity
    {
    private:
        static EntityID entityIdCounter;
    public:
        std::unique_ptr<Event> OnDelete;
        const EntityID entityId;
        explicit Entity()
        : entityId(++entityIdCounter), OnDelete(std::make_unique<Event>())
        {}
        ~Entity()
        {
            OnDelete->InvokeAllCallbacks();
        }
    };
}

