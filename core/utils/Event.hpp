//
// Created by steve on 05/01/2025.
//

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace sage
{
    using SubscriberId = int;

    class EventBase;

    class Connection
    {
        EventBase* event{};
        SubscriberId id = -1;

      public:
        void UnSubscribe();
        explicit Connection(EventBase* _event, SubscriberId _id);
    };

    class EventBase
    {
        virtual void unSubscribe(SubscriberId id) = 0;

      protected:
      public:
        virtual ~EventBase() = default;

        friend class Connection;
    };

    template <typename... Args>
    class Event : public EventBase
    {
        unsigned int count = 0;
        std::unordered_map<unsigned int, std::function<void(Args...)>> subscribers;

        void unSubscribe(SubscriberId id) override
        {
            if (id < 0 || !subscribers.contains(id)) return; // Has already been unsubscribed
            subscribers.erase(id);
        }

      public:
        // [[nodiscard]]
        std::shared_ptr<Connection> Subscribe(std::function<void(Args...)> func)
        {
            auto key = ++count;
            subscribers.emplace(key, func);

            // Shared ptr makes us able to treat connections as polymorphic due to BaseConnection
            return std::make_shared<Connection>(this, key);
        }

        void Publish(Args... args)
        {
            // Make a copy of the subscribers to prevent issues if callbacks modify the subscriber list
            auto subscribersCopy = subscribers;
            for (const auto& [key, subscriber] : subscribersCopy)
            {
                subscriber(args...);
            }
        }
    };

} // namespace sage
