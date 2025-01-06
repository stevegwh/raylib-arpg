//
// Created by steve on 05/01/2025.
//

#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>

namespace sage
{
    using SubscriberId = std::string;

    template <typename... Args>
    class Event;

    class BaseConnection
    {
      public:
        virtual void UnSubscribe() = 0;
        virtual ~BaseConnection() = default;
    };

    template <typename... Args>
    class Connection final : public BaseConnection
    {
        Event<Args...>* event;
        SubscriberId id;

      public:
        void UnSubscribe() override
        {
            if (!event) return;
            event->unSubscribe(id);
            id.clear();
            event = nullptr;
        }

        explicit Connection(Event<Args...>* _event, SubscriberId _id) : event(_event), id(std::move(_id))
        {
        }
    };

    template <typename... Args>
    class Event
    {
        int count = 0;
        std::unordered_map<SubscriberId, std::function<void(Args...)>> subscribers;

        void unSubscribe(SubscriberId id)
        {
            if (id.empty()) return;
            subscribers.erase(id);
        }

      public:
        [[nodiscard]] std::shared_ptr<Connection<Args...>> Subscribe(std::function<void(Args...)> func)
        {
            auto key = std::string(std::to_string(++count));
            subscribers.emplace(key, func);

            return std::make_shared<Connection<Args...>>(this, key);
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

        friend class Connection<Args...>;
    };

} // namespace sage
