#pragma once
#include <algorithm>
#include <cassert>
#include <entt/entt.hpp>
#include <functional>
#include <vector>

namespace sage
{
    class BridgeManager
    {
        class BridgeBase
        {
          public:
            virtual ~BridgeBase() {};
        };

        /*
         * Below offers a way to hook onto mouse events (etc.) and publish the original signals but with a
         * reference to the caller's entity id ("self") so the system can use the entity to get the component data.
         */
        template <typename... Args>
        class EntityEventBridge : public BridgeBase
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
            explicit EntityEventBridge(
                entt::entity subscriber,
                entt::sigh<void(Args...)>& signal,
                entt::sigh<void(entt::entity, Args...)>& _outSignal)
                : subscriber(subscriber)
            {
                entt::sink sink{signal};
                connection = sink.template connect<&EntityEventBridge::onEventPublish>(*this);
                outSignal = &_outSignal;
            }

            ~EntityEventBridge() override
            {
                connection.release();
            }
        };

        std::vector<std::unique_ptr<BridgeBase>> bridges;
        int counter = -1;

      public:
        template <typename... Args>
        int CreateBridge(
            entt::entity entity,
            entt::sigh<void(Args...)>& inSignal,
            entt::sigh<void(entt::entity, Args...)>& outSignal)
        {
            auto bridge = std::make_unique<EntityEventBridge<Args...>>(entity, inSignal, outSignal);
            bridges.push_back(std::move(bridge));
            return ++counter;
        }

        void RemoveBridge(int id)
        {
            bridges[id].reset();
        }
    };

} // namespace sage