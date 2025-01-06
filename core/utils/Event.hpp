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

    class Connection
    {
        EventBase* event{};
        SubscriberId id = -1;

      public:
        void UnSubscribe();
        ~Connection();
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
            if (id < 0 || subscribers.empty() || !subscribers.contains(id))
                return; // Has already been unsubscribed
            subscribers.erase(id);
        }

      public:
        std::unique_ptr<Connection> Subscribe(std::function<void(Args...)> func)
        {
            auto key = ++count;
            subscribers.emplace(key, func);

            return std::make_unique<Connection>(this, key);
        }

        void Publish(Args... args) const
        {
            // Make a copy of the subscribers to prevent issues if callbacks modify the subscriber list
            auto subscribersCopy = subscribers;
            for (const auto& [key, subscriber] : subscribersCopy)
            {
                subscriber(args...);
            }
        }

        Event() = default;
        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;
        // Delete move?

        ~Event() override = default;
    };

} // namespace sage
