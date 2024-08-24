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
        entt::sigh<void(entt::entity, Args...)>* outSignal;

        void onEventPublish(Args... args)
        {
            outSignal->publish(subscriber, args...);
        }

      public:
        explicit EntityEventBridge(entt::entity subscriber) : subscriber(subscriber)
        {
        }

        void BridgeEvents(entt::sigh<void(Args...)>& signal, entt::sigh<void(entt::entity, Args...)>& _outSignal)
        {
            entt::sink sink{signal};
            connection = sink.template connect<&EntityEventBridge::onEventPublish>(*this);
            outSignal = &_outSignal;
        }

        void StopObserving()
        {
            connection.release();
        }

        void DisconnectAll()
        {
            StopObserving();
        }

        ~EntityEventBridge()
        {
            DisconnectAll();
        }
    };

} // namespace sage