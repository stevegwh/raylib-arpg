#pragma once
#include <algorithm>
#include <cassert>
#include <entt/entt.hpp>
#include <functional>
#include <vector>

namespace sage
{
    template <typename... Args>
    class EntityEventBridge
    {
      private:
        entt::entity subscriber;
        entt::connection connection;
        std::function<void(entt::entity, Args...)> callback;

        void onEventPublish(Args... args)
        {
            if (callback)
            {
                callback(subscriber, std::forward<Args>(args)...);
            }
        }

      public:
        explicit EntityEventBridge(entt::entity subscriber) : subscriber(subscriber)
        {
        }

        void Observe(entt::sigh<void(Args...)>& signal)
        {
            entt::sink sink{signal};
            connection = sink.template connect<&EntityEventBridge::onEventPublish>(*this);
        }

        template <typename Instance, auto Func>
        void SetCallback(Instance& instance)
        {
            // assert(callback == nullptr);
            if constexpr (std::is_invocable_v<decltype(Func), Instance&, entt::entity, Args...>)
            {
                callback = [&instance](entt::entity e, Args... args) {
                    (instance.*Func)(e, std::forward<Args>(args)...);
                };
            }
            else if constexpr (std::is_invocable_v<decltype(Func), Instance&, entt::entity>)
            {
                callback = [&instance](entt::entity e, Args...) { (instance.*Func)(e); };
            }
        }

        void RemoveCallback()
        {
            callback = nullptr;
        }

        void StopObserving()
        {
            connection.release();
        }

        void DisconnectAll()
        {
            RemoveCallback();
            StopObserving();
        }

        ~EntityEventBridge()
        {
            DisconnectAll();
        }
    };

} // namespace sage