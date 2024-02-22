//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

namespace sage
{
    typedef int EntityID;
    
    struct Entity
    {
    private:
        static EntityID entityIdCounter;
    public:
        bool deleted = false;
        const EntityID entityId;
        explicit Entity()
        : entityId(++entityIdCounter)
        {}
    };
}

