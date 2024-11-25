#pragma once
#include <algorithm>
#include <cassert>
#include <entt/entt.hpp>
#include <functional>
#include <vector>

namespace sage
{
    /*
     * Offers a way to hook onto mouse events (etc.) and forward the original signals but with a
     * reference to the caller's entity id ("self") so the system can use the entity to get the component data.
     */
    class EntityReflectionSignalRouter
    {
        class ReflectionSignalBase
        {
          public:
            virtual ~ReflectionSignalBase() = default;
        };

        template <typename... Args>
        class EntityReflectionSignalHook : public ReflectionSignalBase
        {
          private:
            entt::entity subscriber;
            entt::connection connection;
            entt::sigh<void(entt::entity, Args...)>* outSignal;

            void onSignalPublish(Args... args)
            {
                outSignal->publish(subscriber, args...);
            }

          public:
            explicit EntityReflectionSignalHook(
                entt::entity subscriber,
                entt::sigh<void(Args...)>& signal,
                entt::sigh<void(entt::entity, Args...)>& _outSignal)
                : subscriber(subscriber)
            {
                entt::sink sink{signal};
                connection = sink.template connect<&EntityReflectionSignalHook::onSignalPublish>(*this);
                outSignal = &_outSignal;
            }

            ~EntityReflectionSignalHook() override
            {
                connection.release();
            }
        };

        std::vector<std::unique_ptr<ReflectionSignalBase>> hooks;
        int counter = -1;

      public:
        template <typename... Args>
        int CreateHook(
            entt::entity entity,
            entt::sigh<void(Args...)>& inSignal,
            entt::sigh<void(entt::entity, Args...)>& outSignal)
        {
            auto bridge = std::make_unique<EntityReflectionSignalHook<Args...>>(entity, inSignal, outSignal);
            hooks.push_back(std::move(bridge));
            return ++counter;
        }

        void RemoveHook(int id)
        {
            if (hooks.size() > id)
            {
                if (hooks[id])
                {
                    hooks[id].reset();
                }
            }
        }
    };

} // namespace sage