//
// Created by Steve Wheeler on 06/12/2024.
//

#pragma once

#include "Event.hpp"

#include "entt/entt.hpp"

#include <memory>

namespace sage
{
    class Systems;

    class CursorClickIndicator
    {
        entt::registry* registry;
        Systems* sys;
        entt::entity self;
        float k = 0;

        std::unique_ptr<Connection> destinationReachedCnx;

        void onCursorClick(entt::entity entity) const;
        void disableIndicator() const;
        void onSelectedActorChanged(entt::entity, entt::entity current);

      public:
        void Update();
        CursorClickIndicator(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
