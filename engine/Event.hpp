//
// Created by steve on 05/01/2025.
//

#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace sage
{
    using SubscriberId = int;

    class EventBase;

    // Wrapper class to track a subscription to an event. Allows easy unsubscribing.
    class Subscription
    {
        EventBase* event = nullptr;
        SubscriberId id = -1;

      public:
        bool IsActive();
        void UnSubscribe();
        ~Subscription();
        explicit Subscription(EventBase* _event, SubscriberId _id);
        Subscription() = default;
    };

    class EventBase
    {
        // Force unsubscribing via the Subscription class
        virtual void unSubscribe(SubscriberId id) = 0;

      public:
        virtual ~EventBase() = default;

        friend class Subscription;
    };

    template <typename... Args>
    class Event : public EventBase
    {
        unsigned int count = 0;
        std::unordered_map<unsigned int, std::function<void(Args...)>> subscriptions;

        void unSubscribe(SubscriberId id) override
        {
            if (id < 0 || subscriptions.empty() || !subscriptions.contains(id))
                return; // Has already been unsubscribed
            subscriptions.erase(id);
        }

      public:
        Subscription Subscribe(std::function<void(Args...)> func)
        {
            auto key = ++count;
            subscriptions.emplace(key, func);

            return Subscription(this, key);
        }

        void Publish(Args... args) const
        {
            // Make a copy of the subscriptions to prevent issues if callbacks modify the subscription list
            if (subscriptions.empty()) return;
            auto subscribersCopy = subscriptions;
            for (const auto& [key, callback] : subscribersCopy)
            {
                callback(args...);
            }
        }

        Event() = default;
        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;
        // Delete move?

        ~Event() override = default;
    };

} // namespace sage
