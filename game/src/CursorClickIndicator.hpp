//
// Created by Steve Wheeler on 06/12/2024.
//

#pragma once

#include "engine/Event.hpp"

#include "entt/entt.hpp"

#include <memory>

namespace lq
{
    class Systems;

    class CursorClickIndicator
    {
        entt::registry* registry;
        Systems* sys;
        entt::entity self;
        float k = 0;

        sage::Subscription destinationReachedSub{};

        void onCursorClick(entt::entity entity) const;
        void disableIndicator() const;
        void onSelectedActorChanged(entt::entity, entt::entity current);

      public:
        void Update();
        CursorClickIndicator(entt::registry* _registry, Systems* _sys);
    };

} // namespace lq
