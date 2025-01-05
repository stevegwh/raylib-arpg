//
// Created by steve on 05/01/2025.
//

#pragma once

#include <functional>
#include <unordered_map>

namespace sage
{

    using SubscriberId = unsigned int;

    template <typename... Args>
    class Event;

    template <typename... Args>
    class Connection
    {
        Event<Args...>* event;
        SubscriberId id = -1;

      public:
        void UnSubscribe()
        {
            event->Unsubscribe(id);
        }

        ~Connection()
        {
            event->Unsubscribe(id);
        }

        explicit Connection(Event<Args...>* _event, const SubscriberId _id) : event(_event), id(_id)
        {
        }

        Connection() = default;
    };

    template <typename... Args>
    class Event
    {
        unsigned int count = 0;
        std::unordered_map<SubscriberId, std::function<void(Args...)>> subscribers;

      public:
        Connection<Args...> Subscribe(std::function<void(Args...)> func)
        {
            subscribers.emplace(++count, func);

            return Connection<Args...>{this, count};
        }

        void Publish(Args... args)
        {
            for (const auto& [key, subscriber] : subscribers)
            {
                subscriber(args...);
            }
        }

        void Unsubscribe(SubscriberId& id)
        {
            if (id == -1) return;
            subscribers.erase(id);
            id = -1;
        }
    };

} // namespace sage
