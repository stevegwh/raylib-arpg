#pragma once

#include "raylib.h"

#include "entt/entt.hpp"
#include "engine/Event.hpp"
#include <vector>

namespace sage
{
    template <typename StateComponentType, typename StateEnum>
    class BaseStateComponent
    {
        StateEnum currentState;
        std::vector<Subscription> currentStateSubscriptions;

      public:
        // All subscriptions added here will be removed when ChangeState is called (via StateMachine).
        void ManageSubscription(const Subscription& newConnection)
        {
            currentStateSubscriptions.push_back(newConnection);
        }

        void RemoveAllSubscriptions()
        {
            for (auto& connection : currentStateSubscriptions)
            {
                connection.UnSubscribe();
            }
            currentStateSubscriptions.clear();
        }

        void SetStateEnum(StateEnum newState)
        {
            currentState = newState;
        }

        [[nodiscard]] StateEnum GetCurrentStateEnum() const
        {
            return currentState;
        }

        ~BaseStateComponent()
        {
            RemoveAllSubscriptions();
        }

        // Allow moving
        BaseStateComponent(BaseStateComponent&& other) noexcept = default;
        BaseStateComponent& operator=(BaseStateComponent&& other) noexcept = default;

        // Prevent copying
        BaseStateComponent(const BaseStateComponent&) = delete;
        BaseStateComponent& operator=(const BaseStateComponent&) = delete;

        explicit BaseStateComponent(StateEnum initialState) : currentState(initialState)
        {
        }
    };

} // namespace sage