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
        bool serializable;
        std::unique_ptr<Event> OnDelete;
        const EntityID entityId;
        explicit Entity(bool _serializable)
        : entityId(++entityIdCounter), OnDelete(std::make_unique<Event>()), serializable(_serializable)
        {}
        ~Entity()
        {
            OnDelete->InvokeAllCallbacks();
        }
    };
}

